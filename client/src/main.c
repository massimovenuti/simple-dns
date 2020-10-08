#include "main.h"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: <path of config file>");
    }

    struct type_addr *tab_addr;
    tab_addr = parse(argv[1]);

    int soc_v4, soc_v6;
    if ((soc_v4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) {
        perror("Error Socket");
        exit(EXIT_FAILURE);
    }
    if ((soc_v6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP)) == -1) {
        perror("Error Socket");
        exit(EXIT_FAILURE);
    }

    char req[512];
    scanf("%s", req);
    for (int i = 0; !tab_addr[i].end; i++) {
        printf("i: %d\nend: %d\n", i, tab_addr[i].end);
        if (tab_addr[i].type == AF_INET) {
            if (sendto(soc_v4, req, strlen(req) + 1, 0, &tab_addr[i].addr, (socklen_t)sizeof(tab_addr[i].addr)) == -1) {
                perror("Error sendto V4");
                exit(EXIT_FAILURE);
            }
        }
        else if (tab_addr[i].type == AF_INET6) {
            if (sendto(soc_v6, req, strlen(req) + 1, 0, &tab_addr[i].addr, (socklen_t)sizeof(tab_addr[i].addr)) == -1) {
                perror("Error sendto V6");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    free(tab_addr);

    exit(EXIT_SUCCESS);
}
