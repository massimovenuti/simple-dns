#include "main.h"

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: <port> <path of config file>");
    }

    int soc_v4, soc_v6;
    if ((soc_v4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) {
        perror("Error Socket V4");
        exit(EXIT_FAILURE);
    }
    if ((soc_v6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP)) == -1) {
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

    if (bind(soc_v4, (struct sockaddr *)&listen_addr_v4, sizeof(listen_addr_v4)) == -1) {
        perror("Error bind V4");
        exit(EXIT_FAILURE);
    }

    if (bind(soc_v6, (struct sockaddr *)&listen_addr_v6, sizeof(listen_addr_v6)) == -1) {
        perror("Error bind V6");
        exit(EXIT_FAILURE);
    }

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
            /* code */
        }
        if (FD_ISSET(soc_v6, &ensemble)) {
            /* code */
        }
        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            scanf("%s", str);
            if (!strcmp(str, "stop")) {
                exit(EXIT_SUCCESS);
            }
        }
    }

    exit(EXIT_SUCCESS);
}
