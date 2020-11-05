#include "main.h"

void ignore(struct addr_with_flag addr, struct ignored_servers *servers) { 
    servers->servers[servers->nb_servers] = addr_to_string(addr);
    servers->nb_servers = (servers->nb_servers + 1) % MAX_IGNORED;
}

char *resolve(int soc, int *id, char *name, struct addr_with_flag *tab_addr, struct ignored_servers ignored_serv, bool free_tab) {
    char req[REQLEN];
    char res[REQLEN];

    struct timeval t;
    PCHK(gettimeofday(&t, NULL));

    if (snprintf(req, REQLEN, "%d|%ld,%ld|%s", *id, t.tv_sec, t.tv_usec, name) > REQLEN - 1)
        fprintf(stderr, "Request too long");

    printf("req: %s\n", req);  //for DEBUG

    struct timeval timeout = {5, 0};
    fd_set ensemble;

    bool find = false;
    bool retry = true;
    do {
        for (int i = 0; !tab_addr[i].end && !find; i++) {
            if (!tab_addr[i].ignore) {
                PCHK(sendto(soc, req, strlen(req) + 1, 0, (struct sockaddr *)&tab_addr[i].addr, (socklen_t)sizeof(struct sockaddr_in6)));

                struct sockaddr_in6 src_addr;
                socklen_t len_addr = sizeof(struct sockaddr_in6);

                FD_ZERO(&ensemble);
                FD_SET(soc, &ensemble);

                PCHK(select(soc + 1, &ensemble, NULL, NULL, &timeout));
                if (FD_ISSET(soc, &ensemble)) {
                    ssize_t len_res;
                    PCHK(len_res = recvfrom(soc, res, REQLEN, 0, (struct sockaddr *)&src_addr, &len_addr));
                    struct res struc_res = parse_res(res, len_res, ignored_serv);
                    printf("res: %s\n", res);
                    printf("in: %lds %ldms\n\n", struc_res.time.tv_sec, struc_res.time.tv_usec/1000);
                    if (struc_res.id == *id) {
                        if (struc_res.code > 0) {
                            find = true;
                            retry = false;
                            if (!strcmp(name, struc_res.name)) {
                                if (free_tab) {
                                    free(tab_addr);
                                }
                                free(struc_res.addrs);
                                return "good";
                            } else {
                                *id = *id + 1;
                                if (free_tab) {
                                    free(tab_addr);
                                }
                                return resolve(soc, id, name, struc_res.addrs, ignored_serv, true);
                            }
                        } else {
                            if (free_tab) {
                                free(tab_addr);
                            }
                            return "Not found";
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
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: <path of config file>");
        exit(EXIT_FAILURE);
    }

    struct addr_with_flag *tab_addr;
    struct ignored_servers ignored_serv;
    ignored_serv.nb_servers = 0;

    tab_addr = parse_conf(argv[1]);

    int soc;
    PCHK(soc = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP));

    bool interactif = true;
    FILE *req_file;
    if (argc == 3) {
        interactif = false;
        MCHK(req_file = fopen(argv[2], "r"));
    }

    int id = 0;
    char name[NAMELEN];
    bool goon = true;
    while (goon && interactif) {
        scanf("%s", name);
        if (*name == '!') {
            if (!strcmp(name, "!stop")) {
                goon = false;
            }

        } else {
            printf("%s\n", resolve(soc, &id, name, tab_addr, ignored_serv, true));
        }
    }

    while (goon && !interactif) {
        if (fscanf(req_file, "%s", name) == EOF) {
            if (ferror(req_file)) {
                perror("fscanf(req_file,\"%s\", name)");
                exit(EXIT_FAILURE);
            } else {
                goon = false;
            }
        } else {
            if (*name == '!') {
                if (!strcmp(name, "!stop")) {
                    goon = false;
                }

            } else {
                printf("%s\n", resolve(soc, &id, name, tab_addr, ignored_serv, true));
            }
        }
    }

    if (!interactif) {
        fclose(req_file);
    }

    free(tab_addr);
    exit(EXIT_SUCCESS);
}
