#include "main.h"

void* processes_request_v4(void* arg) {
    struct thread_arg info = *(struct thread_arg*)arg;
    char req[512];
    struct sockaddr_in src_addr;
    socklen_t len_addr = sizeof(struct sockaddr_in);
    ssize_t len_req;

    PCHK((len_req = recvfrom(info.soc, req, 512, 0, (struct sockaddr*)&src_addr, &len_addr)));
    char* name = parse_req(req, (size_t)len_req);
    size_t len_res = (size_t)len_req;
    char* res = make_res(info.tab_of_addr, req, &len_res);
    PCHK(sendto(info.soc, res, len_res, 0, (struct sockaddr*)&src_addr, len_addr));

    free(name);
    free(res);
    return NULL;
}

void* processes_request_v6(void* arg) {
    struct thread_arg info = *(struct thread_arg*)arg;
    char req[512];
    struct sockaddr_in6 src_addr;
    socklen_t len_addr = sizeof(struct sockaddr_in6);
    ssize_t len_req;

    PCHK((len_req = recvfrom(info.soc, req, 512, 0, (struct sockaddr*)&src_addr, &len_addr)));
    char* name = parse_req(req, (size_t)len_req);
    size_t len_res = (size_t)len_req;
    char* res = make_res(info.tab_of_addr, req, &len_res);
    PCHK(sendto(info.soc, res, len_res, 0, (struct sockaddr*)&src_addr, len_addr));

    free(name);
    free(res);
    return NULL;
}

int main(int argc, char const* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: <port> <path of config file>");
        exit(EXIT_FAILURE);
    }

    int soc_v4, soc_v6;
    PCHK(soc_v4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    PCHK(soc_v6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP));

    struct sockaddr_in listen_addr_v4;
    listen_addr_v4.sin_family = AF_INET;
    listen_addr_v4.sin_addr.s_addr = INADDR_ANY;
    listen_addr_v4.sin_port = htons(atoi(argv[1]));

    int only_v6 = true;
    PCHK(setsockopt(soc_v6, SOL_IPV6, IPV6_V6ONLY, &only_v6, sizeof(int)));

    struct sockaddr_in6 listen_addr_v6;
    listen_addr_v6.sin6_family = AF_INET6;
    listen_addr_v6.sin6_addr = in6addr_any;
    listen_addr_v6.sin6_port = htons(atoi(argv[1]));

    PCHK(bind(soc_v4, (struct sockaddr*)&listen_addr_v4, sizeof(listen_addr_v4)));
    PCHK(bind(soc_v6, (struct sockaddr*)&listen_addr_v6, sizeof(listen_addr_v6)));

    pthread_attr_t thread_attr;
    TCHK(pthread_attr_init(&thread_attr));
    TCHK(pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED));

    pthread_t tid;

    struct name* tab = parse_conf(argv[2]);
    struct thread_arg arg_v4 = {soc_v4, tab};
    struct thread_arg arg_v6 = {soc_v6, tab};

    fd_set ensemble;
    bool goon = true;
    char str[120];

    while (goon) {
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc_v4, &ensemble);
        FD_SET(soc_v6, &ensemble);

        PCHK(select(soc_v6 + 1, &ensemble, NULL, NULL, NULL));

        if (FD_ISSET(soc_v4, &ensemble)) {
            TCHK(pthread_create(&tid, &thread_attr, processes_request_v4, &arg_v4));
        }
        if (FD_ISSET(soc_v6, &ensemble)) {
            TCHK(pthread_create(&tid, &thread_attr, processes_request_v6, &arg_v6));
        }
        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            scanf("%s", str);
            if (!strcmp(str, "stop")) {
                sleep(1);
                TCHK(pthread_attr_destroy(&thread_attr));
                PCHK(close(soc_v4));
                PCHK(close(soc_v6));
                exit(EXIT_SUCCESS);
            }
        }
    }

    exit(EXIT_SUCCESS);
}
