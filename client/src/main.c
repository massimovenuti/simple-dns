#include "main.h"

void ignore(struct addr_with_flag addr, struct ignored_servers *servers) {
    struct server tmp = addr_to_string(addr);
    MCHK(strcpy(servers->servers[servers->nb_servers].ip, tmp.ip));
    MCHK(strcpy(servers->servers[servers->nb_servers].port, tmp.port));
    servers->nb_servers = (servers->nb_servers + 1) % MAX_IGNORED;
}

char *resolve(int soc, int *id, char *name, char *dst, struct addr_with_flag *tab_addr, struct ignored_servers *ignored_serv, bool monitoring, bool free_tab) {
    char req[REQLEN];
    char res[REQLEN];

    struct timeval t;
    PCHK(gettimeofday(&t, NULL));

    if (snprintf(req, REQLEN, "%d|%ld,%ld|%s", *id, t.tv_sec, t.tv_usec, name) > REQLEN - 1)
        fprintf(stderr, "Request too long");

    if (monitoring) fprintf(stderr, "req: %s\n", req);

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
                    struct res struc_res = parse_res(res, len_res, *ignored_serv);

                    if (monitoring) {
                        fprintf(stderr, "res: %s\n", res);
                        fprintf(stderr, "in: %lds %ldms\n\n", struc_res.time.tv_sec, struc_res.time.tv_usec / 1000);
                    }

                    if (struc_res.id == *id) {
                        if (struc_res.code > 0) {
                            find = true;
                            retry = false;
                            if (!strcmp(name, struc_res.name)) {
                                if (free_tab) {
                                    free(tab_addr);
                                }

                                struct server final_res = addr_to_string(*struc_res.addrs);
                                free(struc_res.addrs);

                                snprintf(dst, REQLEN, "%s:%s", final_res.ip, final_res.port);
                                return dst;
                            } else {
                                *id = *id + 1;
                                if (free_tab) {
                                    free(tab_addr);
                                }
                                return resolve(soc, id, name, dst, struc_res.addrs, ignored_serv, monitoring, true);
                            }
                        } else {
                            if (free_tab) {
                                free(tab_addr);
                            }
                            snprintf(dst, REQLEN, "\33[1;31mNot found\033[0m");
                            return dst;
                        }
                    }
                } else if (!retry) {
                    ignore(tab_addr[i], ignored_serv);
                    tab_addr[i].ignore = true;
                }
            }
        }
    } while (retry && !(retry = false));

    if (free_tab) {
        free(tab_addr);
    }

    snprintf(dst, REQLEN, "\33[1;31mTimeout\033[0m");
    return dst;
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
    char res[NAMELEN];
    bool monitoring = false;
    bool goon = true;
    while (goon && interactif) {
        putchar('>');
        scanf("%s", name);
        if (*name == '!') {
            if (!strcmp(name, "!stop")) {
                goon = false;
            } else if (!strcmp(name, "!monitoring")) {
                monitoring = !monitoring;
                if (monitoring) {
                    fprintf(stderr, "monitoring:enabel\n");
                } else {
                    fprintf(stderr, "monitoring:disabel\n");
                }
            } else if (!strcmp(name, "!ignored")) {
                fprintf(stderr, "ignored server:\n");
                for (int i = 0; i < ignored_serv.nb_servers; i++) {
                    fprintf(stderr, "%s:%s\n", ignored_serv.servers[0].ip, ignored_serv.servers[0].port);
                }
                fprintf(stderr, "\n");
            }
        } else {
            printf("%s\n", resolve(soc, &id, name, res, tab_addr, &ignored_serv, monitoring, false));
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
                } else if (!strcmp(name, "!monitoring")) {
                    monitoring = !monitoring;
                    if (monitoring) {
                        fprintf(stderr, "monitoring:enabel");
                    } else {
                        fprintf(stderr, "monitoring:disabel");
                    }
                } else if (!strcmp(name, "!ignored")) {
                    fprintf(stderr, "ignored server:\n");
                    for (int i = 0; i < ignored_serv.nb_servers; i++) {
                        fprintf(stderr, "%s:%s\n", ignored_serv.servers[0].ip, ignored_serv.servers[0].port);
                    }
                    fprintf(stderr, "\n");
                }
            } else {
                printf("%s, %s\n", name, resolve(soc, &id, name, res, tab_addr, &ignored_serv, monitoring, false));
            }
        }
    }

    if (!interactif) {
        fclose(req_file);
    }

    free(tab_addr);
    exit(EXIT_SUCCESS);
}
