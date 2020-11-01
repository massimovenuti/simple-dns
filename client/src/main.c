#include "main.h"

char *resolve(int soc_v4, int soc_v6, int *id, char *name, struct addr_with_flag *tab_addr, bool free_tab) {
    char req[REQ_MAX];
    char res[REQ_MAX];

    time_t t;
    PCHK(time(&t));

    if (snprintf(req, REQ_MAX, "%d|%ld|%s", *id, t, name) > REQ_MAX - 1)
        fprintf(stderr, "Request so long");

    printf("%s\n", req);  //for DEBUG

    struct timeval timeout = {10, 0};
    fd_set ensemble;

    bool find = false;
    bool retry = true;
    do {
        for (int i = 0; !tab_addr[i].end && !find; i++) {
            if (!tab_addr[i].ignore) {
                if (tab_addr[i].addr.sa_family == AF_INET) {
                    PCHK(sendto(soc_v4, req, strlen(req) + 1, 0, &tab_addr[i].addr, (socklen_t)sizeof(tab_addr[i].addr)));

                    struct sockaddr_in src_addr;
                    socklen_t len_addr = sizeof(struct sockaddr_in);

                    FD_ZERO(&ensemble);
                    FD_SET(soc_v4, &ensemble);

                    PCHK(select(soc_v4 + 1, &ensemble, NULL, NULL, &timeout));
                    if (FD_ISSET(soc_v4, &ensemble)) {
                        ssize_t len_res;
                        PCHK(len_res = recvfrom(soc_v4, res, REQ_MAX, 0, (struct sockaddr *)&src_addr, &len_addr));
                        struct res struc_res = parse_res(res, len_res);
                        if (struc_res.id == *id) {
                            if (struc_res.code > 0) {
                                find = true;
                                if (!strcmp(name, struc_res.name)) {
                                    return "good";
                                } else {
                                    *id = *id + 1;
                                    return resolve(soc_v4, soc_v6, id, name, struc_res.addrs, false);
                                }
                            } else {
                                return "Not found";
                            }
                        }
                    }
                } else if (tab_addr[i].addr.sa_family == AF_INET6) {
                    PCHK(sendto(soc_v6, req, strlen(req) + 1, 0, &tab_addr[i].addr, (socklen_t)sizeof(tab_addr[i].addr)));

                    struct sockaddr_in6 src_addr;
                    socklen_t len_addr = sizeof(struct sockaddr_in);
                    ssize_t len_res;

                    FD_ZERO(&ensemble);
                    FD_SET(soc_v6, &ensemble);

                    PCHK(select(soc_v6 + 1, &ensemble, NULL, NULL, &timeout));
                    if (FD_ISSET(soc_v6, &ensemble)) {
                        PCHK((len_res = recvfrom(soc_v6, res, REQ_MAX, 0, (struct sockaddr *)&src_addr, &len_addr)));
                         struct res struc_res = parse_res(res, len_res);
                        if (struc_res.id == *id) {
                            if (struc_res.code != -1) {
                                find = true;
                                if (!strcmp(name, struc_res.name)) {
                                    return "good";
                                } else {
                                    *id = *id + 1;
                                    return resolve(soc_v4, soc_v6, id, name, struc_res.addrs, false);
                                }
                            } else {
                                return "Not found";
                            }
                        }
                    }
                }
            }
        }
    } while (retry && !(retry = false));

    if (free_tab) {
        free(tab_addr);
    }

    return "Not found";
}

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
    scanf("%s", name);
    int id = 0;
    printf("%s\n", resolve(soc_v4, soc_v6, &id, name, tab_addr, true));
    exit(EXIT_SUCCESS);
}
