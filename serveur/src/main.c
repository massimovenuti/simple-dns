#include "main.h"

/**
 * @brief 
 * 
 * @param arg 
 * @return void* 
 */
void processes_request(int soc, struct name *tab_of_addr, lack *ack_wait) {
    struct sockaddr_in6 src_addr;
    char req[REQLEN];
    size_t alloc_mem = RESLEN * sizeof(char);
    char* res = malloc(alloc_mem);
    socklen_t len_addr = sizeof(struct sockaddr_in6);
    ssize_t len_req;
    size_t len_res;
    struct req s_req;
    int id;

    PCHK((len_req = recvfrom(soc, req, 512, 0, (struct sockaddr*)&src_addr, &len_addr)));
    if ((id = is_ack(req)) > -1) {
        *ack_wait = lack_rm(*ack_wait, id, src_addr);
    }else if ((s_req = parse_req(req)).id > -1) {
        *ack_wait = lack_add(*ack_wait, s_req.id, src_addr);
        res = make_res(res, s_req, tab_of_addr, &len_res, len_req, &alloc_mem);
        PCHK(sendto(soc, res, len_res, 0, (struct sockaddr*)&src_addr, len_addr));
    }
    free(res);

    return;
}

int main(int argc, char const* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: <port> <path of config file>");
        exit(EXIT_FAILURE);
    }

    int soc;
    PCHK(soc = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP));

    struct sockaddr_in6 listen_addr_v6;
    listen_addr_v6.sin6_family = AF_INET6;
    listen_addr_v6.sin6_addr = in6addr_any;
    listen_addr_v6.sin6_port = htons(atoi(argv[1]));

    PCHK(bind(soc, (struct sockaddr*)&listen_addr_v6, sizeof(listen_addr_v6)));

    struct name* tab = parse_conf(argv[2]);

    lack ack_wait = lack_new();

    fd_set ensemble;
    bool goon = true;
    char str[120];

    while (goon) {
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);

        PCHK(select(soc + 1, &ensemble, NULL, NULL, NULL));

        if (FD_ISSET(soc, &ensemble)) {
           processes_request(soc, tab, &ack_wait);
        }
        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            scanf("%s", str);
            if (!strcmp(str, "stop")) {
                PCHK(close(soc));
                free_names(tab);
                lack_destroy(ack_wait);
                exit(EXIT_SUCCESS);
            }
        }
    }

    exit(EXIT_SUCCESS);
}
