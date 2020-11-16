
/**
 * @file main.c
 * @author Alexandre Vogel, Massimo Venuti
 * @brief Serveur de nom DNS - fichier source
 * @date 2020-11-16
 *
 */
#include "main.h"

void
processes_request (int soc, struct tab_names tab_of_addr, lack * ack_wait)
{
    struct sockaddr_in6 src_addr;
    char req[REQLEN];
    size_t max_len_res = REQLEN * sizeof (char);
    char *res = malloc (max_len_res);
    socklen_t len_addr = sizeof (struct sockaddr_in6);
    ssize_t len_req;
    size_t len_res;
    struct req s_req;
    int id;

    PCHK ((len_req = recvfrom (soc, req, 512, 0, (struct sockaddr *) &src_addr,
			       &len_addr)));
    req[len_req] = 0;
    if ((id = is_ack (req)) > -1)
    {
	*ack_wait = larm (*ack_wait, id, src_addr);
    }
    else if ((s_req = parse_req (req)).id > -1)
    {
	*ack_wait = laadd (*ack_wait, s_req, src_addr);
	res =
	    make_res (res, s_req, tab_of_addr, &len_res, len_req, &max_len_res);
	PCHK (sendto (soc, res, len_res, 0, (struct sockaddr *) &src_addr,
		      len_addr));
    }
    free (res);

    return;
}

bool
timeout (struct ack a)
{
    struct timeval cur_t;

    PCHK (gettimeofday (&cur_t, NULL));
    cur_t.tv_sec -= TIMEOUT;
    return timercmp (&cur_t, &a.time, >=);
}

void
check_ack (lack * l, int soc, struct tab_names tab_of_addr)
{
    if (laempty (*l))
    {
	return;
    }

    size_t alloc_mem = RESLEN * sizeof (char);
    char *res = malloc (alloc_mem);
    size_t len_res;

    lack tmp;

    for (tmp = *l; !laempty (tmp); tmp = (laempty (tmp)) ? tmp : tmp->next)
    {
	if (timeout (tmp->ack))
	{
	    res = make_res (res, tmp->ack.req, tab_of_addr, &len_res, 0,
			    &alloc_mem);
	    PCHK (sendto
		  (soc, res, len_res, 0, (struct sockaddr *) &tmp->ack.addr,
		   sizeof (struct sockaddr_in6)));
	    tmp->ack.retry++;
	    if (tmp->ack.retry > 3)
	    {
		*l = larm (*l, tmp->ack.req.id, tmp->ack.addr);
		tmp = *l;
	    }
	    else
	    {
		PCHK (gettimeofday (&tmp->ack.time, NULL));
	    }
	}
    }

    free (res);
}

int
main (int argc, char const *argv[])
{
    if (argc != 3)
    {
	fprintf (stderr, "Usage: <port> <path of config file>");
	exit (EXIT_FAILURE);
    }

    int soc;

    PCHK (soc = socket (AF_INET6, SOCK_DGRAM, IPPROTO_UDP));

    struct sockaddr_in6 listen_addr_v6;

    listen_addr_v6.sin6_family = AF_INET6;
    listen_addr_v6.sin6_addr = in6addr_any;
    listen_addr_v6.sin6_port = htons (atoi (argv[1]));

    PCHK (bind
	  (soc, (struct sockaddr *) &listen_addr_v6, sizeof (listen_addr_v6)));

    struct tab_names tab = parse_conf (argv[2]);

    lack ack_wait = lanew ();

    fd_set ensemble;
    bool goon = true;
    char str[120];

    struct timeval timeout_loop;

    while (goon)
    {
	timeout_loop.tv_sec = 1;
	timeout_loop.tv_usec = 0;
	FD_ZERO (&ensemble);
	FD_SET (STDIN_FILENO, &ensemble);
	FD_SET (soc, &ensemble);

	PCHK (select (soc + 1, &ensemble, NULL, NULL, &timeout_loop));

	if (FD_ISSET (soc, &ensemble))
	{
	    processes_request (soc, tab, &ack_wait);
	}
	if (FD_ISSET (STDIN_FILENO, &ensemble))
	{
	    scanf ("%s", str);
	    if (!strcmp (str, "stop"))
	    {
		PCHK (close (soc));
		free_tab_names (&tab);
		ladestroy (ack_wait);
		exit (EXIT_SUCCESS);
	    }
	}
	check_ack (&ack_wait, soc, tab);
    }

    exit (EXIT_SUCCESS);
}
