#include "main.h"

void* processes_request(void* arg) {
    struct thread_arg info = *(struct thread_arg*)arg;
    char req[REQLEN];
    char res[REQLEN];
    struct sockaddr_in6 src_addr;
    socklen_t len_addr = sizeof(struct sockaddr_in6);
    ssize_t len_req;

    PCHK((len_req = recvfrom(info.soc, req, 512, 0, (struct sockaddr*)&src_addr, &len_addr)));
    size_t len_res = (size_t)len_req;
    make_res(res, req, info.tab_of_addr, &len_res);
    PCHK(sendto(info.soc, res, len_res, 0, (struct sockaddr*)&src_addr, len_addr));

    (void) res;
    (void) req;
    (void) len_res;
    return NULL;
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

    pthread_attr_t thread_attr;
    TCHK(pthread_attr_init(&thread_attr));
    TCHK(pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED));

    pthread_t tid;

    struct name* tab = parse_conf(argv[2]);
    struct thread_arg arg = {soc, tab};

    fd_set ensemble;
    bool goon = true;
    char str[120];

    while (goon) {
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);

        PCHK(select(soc + 1, &ensemble, NULL, NULL, NULL));

        if (FD_ISSET(soc, &ensemble)) {
            TCHK(pthread_create(&tid, &thread_attr, processes_request, &arg));
        }
        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            scanf("%s", str);
            if (!strcmp(str, "stop")) {
                TCHK(pthread_attr_destroy(&thread_attr));
                PCHK(close(soc));
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_SUCCESS;
}
