#include "main.h"

void ignore(struct addr_with_flag addr, struct ignored_servers *servers) {
    struct server tmp = addr_to_string(addr);
    MCHK(strcpy(servers->servers[servers->nb_servers].ip, tmp.ip));
    MCHK(strcpy(servers->servers[servers->nb_servers].port, tmp.port));
    servers->nb_servers = (servers->nb_servers + 1) % MAX_IGNORED;
}

// ajouter un flag "check_sons"
//on peut threader cette partie
char *resolve(int soc, struct request *request, char *dst, struct tree *tree_addr, struct ignored_servers *ignored_serv, bool monitoring, bool free_tab) {    
    struct tree *t_rech = rech(request->name, tree_addr);      // on regarde si on a pas déjà fait une requête similaire

    if (!strcmp(t_rech->name, request->name)) {                 // si on a déjà trouvé cette adresse, on renvoie directement le résultat
        struct server final_res = addr_to_string(*t_rech->tab_addr);
        snprintf(dst, REQLEN, "%s:%s", final_res.ip, final_res.port);
        return dst;
    }
    else if (strcmp(t_rech->name, tree_addr->name)) {           // si on a déjà trouvé des domaines, sous-domaines, etc on reprend de la
        return resolve(soc, request, dst, t_rech, ignored_serv, monitoring, free_tab);
    }
    else {
        char req[REQLEN];
        char res[REQLEN];

        struct timeval t;
        PCHK(gettimeofday(&t, NULL));

        if (snprintf(req, REQLEN, "%d|%ld,%ld|%s", request->id, t.tv_sec, t.tv_usec, request->name) > REQLEN - 1)
            fprintf(stderr, "Request too long");

        if (monitoring) 
            fprintf(stderr, "req: %s\n", req);

        struct timeval timeout = {5, 0};

        fd_set ensemble;

        bool find = false;
        bool retry = true;
        do {
            for (int count = 0, j = tree_addr->index; count < tree_addr->nb_addrs && !find; count++, j = (j + 1) % tree_addr->nb_addrs) {
                tree_addr->index = j;
                struct server tmp_serv = addr_to_string(tree_addr->tab_addr[j]);
                if (!is_ignored(tmp_serv.ip, tmp_serv.port, *ignored_serv)) {
                    PCHK(sendto(soc, req, strlen(req) + 1, 0, (struct sockaddr *)&tree_addr->tab_addr[j].addr, (socklen_t)sizeof(struct sockaddr_in6)));

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

                        if (struc_res.id == request->id) {
                            if (struc_res.code > 0) {
                                find = true;
                                retry = false;
                                if (!strcmp(request->name, struc_res.name)) {
                                    /*
                                    if (free_tab) {
                                        free(tab_addr);
                                    }
                                    */
                                    struct server final_res = addr_to_string(*struc_res.addrs);
                                    free(struc_res.addrs);

                                    snprintf(dst, REQLEN, "%s:%s", final_res.ip, final_res.port);
                                    return dst;
                                } else {
                                    request->id += 1;
                                    /*
                                    if (free_tab) {
                                        free(tab_addr);
                                    }
                                    */       
                                    struct tree new_tree_addr; 

                                    strcpy(new_tree_addr.name, struc_res.name);
                                    int count;
                                    for (count = 0; count < MAX_ADDR && !struc_res.addrs[count].end; count++) {
                                        new_tree_addr.tab_addr[count] = struc_res.addrs[count];
                                    }
                                    MCHK(new_tree_addr.sons = malloc(MAX_SONS * sizeof(struct tree)));
                                    new_tree_addr.nb_sons = 0;
                                    new_tree_addr.nb_addrs = count;
                                    new_tree_addr.index = 0;

                                    if (tree_addr->nb_sons == 0) {
                                        tree_addr->sons[0] = new_tree_addr;
                                    } else {
                                        tree_addr->sons[(tree_addr->nb_sons + 1) % tree_addr->nb_sons] = new_tree_addr;
                                    }
                                    tree_addr->nb_sons = (tree_addr->nb_sons == MAX_SONS) ? MAX_SONS : tree_addr->nb_sons + 1;

                                    return resolve(soc, request, dst, &new_tree_addr, ignored_serv, monitoring, true);
                                }
                            } else {
                                /*
                                if (free_tab) {
                                    free(tab_addr);
                                }
                                */
                                snprintf(dst, REQLEN, "\33[1;31mNot found\033[0m");
                                return dst;
                            }
                        }
                    } else if (!retry) {
                        ignore(tree_addr->tab_addr[j], ignored_serv);
                    }
                }
            }
        } while (retry && !(retry = false));
        /*
        if (free_tab) {
            free(tab_addr);
        }
        */
        snprintf(dst, REQLEN, "\33[1;31mTimeout\033[0m");
    }

    return dst;
}

int main(int argc, char const *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: <path of config file>");
        exit(EXIT_FAILURE);
    }

    struct addr_with_flag *tab_addr;
    struct ignored_servers ignored_serv;
    struct request request;
    struct tree tree_addr;
    ignored_serv.nb_servers = 0;

    tab_addr = parse_conf(argv[1]);

    int i;
    for (i = 0; !tab_addr[i].end; i++) {
        tree_addr.tab_addr[i] = tab_addr[i];
    }

    tree_addr.nb_addrs = i;
    tree_addr.nb_sons = 0;
    tree_addr.index = 0;
    MCHK(tree_addr.sons = malloc(MAX_SONS * sizeof(struct tree)));
    strcpy(tree_addr.name, "\0");

    int soc;
    PCHK(soc = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP));

    bool interactif = true;
    FILE *req_file;
    if (argc == 3) {
        interactif = false;
        MCHK(req_file = fopen(argv[2], "r"));
    }

    //int id = 0;
    //char name[NAMELEN];
    char res[NAMELEN];
    bool monitoring = false;
    bool goon = true;
    while (goon && interactif) {
        putchar('>');
        //...
        scanf("%s", request.name);
        request.id = 0;
        if (*request.name != '!') {
            //...
            printf("%s, %s\n", request.name, resolve(soc, &request, res, &tree_addr, &ignored_serv, monitoring, false));
        } else {
            if (!strcmp(request.name, "!stop")) {
                goon = false;
            } else if (!strcmp(request.name, "!monitoring")) {
                monitoring = !monitoring;
                if (monitoring) {
                    fprintf(stderr, "monitoring:enabel\n");
                } else {
                    fprintf(stderr, "monitoring:disabel\n");
                }
            } else if (!strcmp(request.name, "!ignored")) {
                fprintf(stderr, "ignored server:\n");
                for (int i = 0; i < ignored_serv.nb_servers; i++) {
                    fprintf(stderr, "%s:%s\n", ignored_serv.servers[0].ip, ignored_serv.servers[0].port);
                }
                fprintf(stderr, "\n");
            }
        }
    }

    while (goon && !interactif) {
        if (fscanf(req_file, "%s", request.name) == EOF) {
            if (ferror(req_file)) {
                perror("fscanf(req_file,\"%s\", name)");
                exit(EXIT_FAILURE);
            } else {
                goon = false;
            }
        } else {
            if (*request.name != '!') {
                //...
                printf("%s, %s\n", request.name, resolve(soc, &request, res, &tree_addr, &ignored_serv, monitoring, false));
            } else {
                if (!strcmp(request.name, "!stop")) {
                    goon = false;
                } else if (!strcmp(request.name, "!monitoring")) {
                    monitoring = !monitoring;
                    if (monitoring) {
                        fprintf(stderr, "monitoring:enabel");
                    } else {
                        fprintf(stderr, "monitoring:disabel");
                    }
                } else if (!strcmp(request.name, "!ignored")) {
                    fprintf(stderr, "ignored server:\n");
                    for (int i = 0; i < ignored_serv.nb_servers; i++) {
                        fprintf(stderr, "%s:%s\n", ignored_serv.servers[0].ip, ignored_serv.servers[0].port);
                    }
                    fprintf(stderr, "\n");
                }
            }
        }
    }

    if (!interactif) {
        fclose(req_file);
    }

    free(tab_addr);
    //free...
    exit(EXIT_SUCCESS);
}
