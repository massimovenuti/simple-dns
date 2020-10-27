#include "main.h"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: <path of config file>");
    }

    struct addr_with_flag *tab_addr;
    tab_addr = parse_conf(argv[1]);

    int soc_v4, soc_v6;
    if ((soc_v4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) {
        perror("Error Socket");
        exit(EXIT_FAILURE);
    }
    if ((soc_v6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP)) == -1) {
        perror("Error Socket");
        exit(EXIT_FAILURE);
    }

    char name[NAME_MAX];
    char req[REQ_MAX];
    int id = 1;

    scanf("%s", name);

    time_t t;
    if (time(&t) == -1) {
        perror("Error time");
        exit(EXIT_FAILURE);
    }

    if (snprintf(req, 512, "%d|%ld|%s", id, t, name) > REQ_MAX - 1) {
        fprintf(stderr, "Request so long");
    }
    printf("%s\n", req);  //for DEBUG

    for (int i = 0; !tab_addr[i].end; i++) {
        if (!tab_addr[i].ignore) {
            if (tab_addr[i].addr.sa_family == AF_INET) {
                if (sendto(soc_v4, req, strlen(req) + 1, 0, &tab_addr[i].addr, (socklen_t)sizeof(tab_addr[i].addr)) == -1) {
                    perror("Error sendto V4");
                    exit(EXIT_FAILURE);
                }
            } else if (tab_addr[i].addr.sa_family == AF_INET6) {
                if (sendto(soc_v6, req, strlen(req) + 1, 0, &tab_addr[i].addr, (socklen_t)sizeof(tab_addr[i].addr)) == -1) {
                    perror("Error sendto V6");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    free(tab_addr);

    exit(EXIT_SUCCESS);
}
