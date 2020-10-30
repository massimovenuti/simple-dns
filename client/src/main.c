#include "main.h"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: <path of config file>");
        exit(EXIT_FAILURE);
    }

    struct addr_with_flag *tab_addr;
    tab_addr = parse_conf(argv[1]);

    int soc_v4, soc_v6;
    PCHK(soc_v4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP));
    PCHK(soc_v6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP));

    char name[NAME_MAX];
    char req[REQ_MAX];
    int id = 1;

    scanf("%s", name);

    time_t t;
    PCHK(time(&t));

    if (snprintf(req, 512, "%d|%ld|%s", id, t, name) > REQ_MAX - 1)
        fprintf(stderr, "Request so long");

    printf("%s\n", req);  //for DEBUG

    for (int i = 0; !tab_addr[i].end; i++) {
        if (!tab_addr[i].ignore) {
            if (tab_addr[i].addr.sa_family == AF_INET)
                PCHK(sendto(soc_v4, req, strlen(req) + 1, 0, &tab_addr[i].addr, (socklen_t)sizeof(tab_addr[i].addr)));
            else if (tab_addr[i].addr.sa_family == AF_INET6)
                PCHK(sendto(soc_v6, req, strlen(req) + 1, 0, &tab_addr[i].addr, (socklen_t)sizeof(tab_addr[i].addr)));
        }
    }
    free(tab_addr);
    exit(EXIT_SUCCESS);
}
