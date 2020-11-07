#include "parser.h"

void convert(char ip[], int port, struct sockaddr_in6 *dst) {
    int check;
    struct sockaddr_in *ipV4addr = (struct sockaddr_in *)(dst);
    PCHK(check = inet_pton(AF_INET, ip, &ipV4addr->sin_addr));

    if (check == 0) {
        PCHK(check = inet_pton(AF_INET6, ip, &dst->sin6_addr));
        if (check == 0) {
            fprintf(stderr, "Invalid address: %s\n", ip);
            exit(EXIT_FAILURE);
        }
        dst->sin6_port = htons(port);
        dst->sin6_family = AF_INET6;
    } else {
        ipV4addr->sin_port = htons(port);
        ipV4addr->sin_family = AF_INET;
    }
}

bool is_ignored(char *ip, char *port, struct ignored_servers servers) {
    int i;
    for (i = 0; i < servers.nb_servers && strcmp(ip, servers.servers[i].ip) != 0 && strcmp(port, servers.servers[i].port) != 0; i++)
        ;
    return i != servers.nb_servers;
}

struct tab_addr parse_conf(const char *file_name) {
    struct tab_addr res;
    int max_addrs = TABSIZE;

    MCHK(res.addr = malloc(max_addrs * sizeof(struct sockaddr_in6)));

    FILE *file;

    MCHK(file = fopen(file_name, "r"));

    char ip[IPLEN];
    int port;

    int i;
    for (i = 0; fscanf(file, "%[^|- ] | %d\n", ip, &port) != EOF; i++) {
        if (i >= max_addrs) {
            max_addrs *= INCREASE_COEF;
            MCHK(res.addr = realloc(res.addr, max_addrs * sizeof(struct sockaddr_in6)));
        }
        convert(ip, port, &res.addr[i]);
    }
    res.len = i + 1;

    PCHK(fclose(file));
    return res;
}

struct res parse_res(char *res, size_t len, struct ignored_servers servers) {
    (void)len;
    struct res s_res;
    char tab_addr[1024];
    *tab_addr = '\0';

    if (sscanf(res, " %d | %ld,%ld | %[^|- ] | %d | %s", &s_res.id, &s_res.time.tv_sec, &s_res.time.tv_usec, s_res.req_name, &s_res.code, tab_addr) < 4) {
        fprintf(stderr, "Incorrect server result\n");
        exit(EXIT_FAILURE);
    }

    struct timeval t;
    PCHK(gettimeofday(&t, NULL));

    s_res.time.tv_sec = t.tv_sec - s_res.time.tv_sec;
    s_res.time.tv_usec = t.tv_usec - s_res.time.tv_usec;

    char *token;
    char name[NAMELEN];
    char ip[IPLEN];
    char port[PORTLEN];
    int nb_addrs = 0;
    int max_addrs = TABSIZE;
    bool first = true;

    token = strtok(tab_addr, SEPARATOR);

    if (token == NULL) {
        s_res.addrs.addr = NULL;
        s_res.addrs.len = 0;
    } else {
        s_res.addrs.addr = malloc(max_addrs * sizeof(struct sockaddr_in6));
        do {
            if (sscanf(token, " %[^, ] , %[^, ] , %[^, ] ", name, ip, port) != 3) {
                fprintf(stderr, "Server result incorrect\n");
                exit(EXIT_FAILURE);
            }
            if (first) {
                MCHK(strcpy(s_res.name, name));
                first = false;
            }
            if (nb_addrs > max_addrs) {
                max_addrs *= INCREASE_COEF;
                MCHK(s_res.addrs.addr = realloc(s_res.addrs.addr, max_addrs * sizeof(struct sockaddr_in6)));
            }
            if (!is_ignored(ip, port, servers)) {
                convert(ip, atoi(port), &s_res.addrs.addr[nb_addrs]);
                nb_addrs++;
            }
            token = strtok(NULL, SEPARATOR);
        } while (token != NULL);
        s_res.addrs.len = nb_addrs + 1;
    }

    return s_res;
}

struct server addr_to_string(struct sockaddr_in6 addr) {
    struct server res;
    sprintf(res.port, "%d", ntohs(addr.sin6_port));
    if (addr.sin6_family == AF_INET6) {
        inet_ntop(AF_INET6, &addr.sin6_addr, res.ip, sizeof(addr));
    } else {
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&addr))->sin_addr, res.ip, sizeof(addr));
    }
    return res;
}
