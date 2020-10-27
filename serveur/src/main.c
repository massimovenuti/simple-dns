#include "main.h"

void* processes_request_v4(void* arg) {
    struct thread_arg info = *(struct thread_arg*)arg;
    char req[512];
    struct sockaddr_in src_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    if (recvfrom(info.soc, req, 512, 0, (struct sockaddr*)&src_addr, &len) == -1) {
        perror("Error recvfrom V4");
        exit(EXIT_FAILURE);
    }

    if (sendto(info.soc, req, 512, 0, (struct sockaddr*)&src_addr, len) == -1) {
        perror("Error sendto V4");
        exit(EXIT_FAILURE);
    }

    printf("ok v4\n"); //for DEBUG
    return NULL;
}

void* processes_request_v6(void* arg) {
    struct thread_arg info = *(struct thread_arg*)arg;
    char req[512];
    struct sockaddr_in6 src_addr;
    socklen_t len = sizeof(struct sockaddr_in6);
    if (recvfrom(info.soc, req, 512, 0, (struct sockaddr*)&src_addr, &len) == -1) {
        perror("Error recvfrom V6");
        exit(EXIT_FAILURE);
    }

    if (sendto(info.soc, req, 512, 0, (struct sockaddr*)&src_addr, len) == -1) {
        perror("Error sendto V6"); //for DEBUG
        exit(EXIT_FAILURE);
    }

    printf("ok v6\n");
    return NULL;
}

int main(int argc, char const* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: <port> <path of config file>");
        exit(EXIT_FAILURE);
    }

    int soc_v4, soc_v6;
    if ((soc_v4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Error Socket V4");
        exit(EXIT_FAILURE);
    }
    if ((soc_v6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Error Socket V6");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in listen_addr_v4;
    listen_addr_v4.sin_family = AF_INET;
    listen_addr_v4.sin_addr.s_addr = INADDR_ANY;
    listen_addr_v4.sin_port = htons(atoi(argv[1]));

    int only_v6 = true;
    if (setsockopt(soc_v6, SOL_IPV6, IPV6_V6ONLY, &only_v6, sizeof(int)) == -1) {
        perror("Error setsockopt");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in6 listen_addr_v6;
    listen_addr_v6.sin6_family = AF_INET6;
    listen_addr_v6.sin6_addr = in6addr_any;
    listen_addr_v6.sin6_port = htons(atoi(argv[1]));

    if (bind(soc_v4, (struct sockaddr*)&listen_addr_v4, sizeof(listen_addr_v4)) == -1) {
        perror("Error bind V4");
        exit(EXIT_FAILURE);
    }

    if (bind(soc_v6, (struct sockaddr*)&listen_addr_v6, sizeof(listen_addr_v6)) == -1) {
        perror("Error bind V6");
        exit(EXIT_FAILURE);
    }

    pthread_attr_t thread_attr;
    if ((errno = pthread_attr_init(&thread_attr)) > 0) {
        perror("pthread_attr_init");
        exit(EXIT_FAILURE);
    }
    if ((errno = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED)) > 0) {
        perror("pthread_attr_setdetachstate");
        exit(EXIT_FAILURE);
    }

    pthread_t tid;

    void* tab = NULL; //modifiter type
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

        if (select(soc_v6 + 1, &ensemble, NULL, NULL, NULL) == -1) {
            perror("Error select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(soc_v4, &ensemble)) {
            if ((errno = pthread_create(&tid, &thread_attr, processes_request_v4, &arg_v4)) > 0) {
                perror("pthread_attr_setdetachstate");
                exit(EXIT_FAILURE);
            }
        }
        if (FD_ISSET(soc_v6, &ensemble)) {
            if ((errno = pthread_create(&tid, &thread_attr, processes_request_v6, &arg_v6)) > 0) {
                perror("pthread_attr_setdetachstate");
                exit(EXIT_FAILURE);
            }
        }
        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            scanf("%s", str);
            if (!strcmp(str, "stop")) {
                sleep(1);
                if ((errno = pthread_attr_destroy(&thread_attr)) > 0) {
                    perror("pthread_attr_destroy");
                    exit(EXIT_FAILURE);
                }

                if (close(soc_v4) == -1) {
                    perror("close V4");
                    exit(EXIT_FAILURE);
                }

                if (close(soc_v6) == -1) {
                    perror("close V6");
                    exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS);
            }
        }
    }

    exit(EXIT_SUCCESS);
}
