#include "main.h"

void ignore(struct addr_with_flag addr, struct ignored_servers *servers) {
    struct server tmp = addr_to_string(addr);
    MCHK(strcpy(servers->servers[servers->nb_servers].ip, tmp.ip));
    MCHK(strcpy(servers->servers[servers->nb_servers].port, tmp.port));
    servers->nb_servers = (servers->nb_servers + 1) % MAX_IGNORED;
}

tree new_tree(char *name, struct addr_with_flag *addrs) {
    tree t;
    MCHK(t = malloc(sizeof(node)));
    for (int i = 0; i < MAX_SONS; i++) {
        t->sons[i] = NULL;
    }
    int count;
    for (count = 0; count < MAX_ADDR && !addrs[count].end; count++) {
        t->tab_addr[count] = addrs[count];
    }
    t->nb_addrs = count;
    t->nb_sons = 0;
    t->index = 0;
    MCHK(strcpy(t->name, name));
    return t;
}

tree adj_son(tree t, char *name, struct addr_with_flag *addrs) {
    if (t == NULL)
        return NULL;
    tree son = new_tree(name, addrs);
    if (t->nb_sons == 0) {
        t->sons[0] = son;
    } else {
        destroy(t->sons[(t->nb_sons) % MAX_SONS]);
        t->sons[(t->nb_sons) % MAX_SONS] = son;
    }
    t->nb_sons = (t->nb_sons == MAX_SONS) ? MAX_SONS : t->nb_sons + 1;
    return son;
}

void destroy(tree t) {
    if (t == NULL) {
        return;
    }
    for (int i = 0; i < t->nb_sons; i++) {
        destroy(t->sons[i]);
    }
    free(t);
    return;
}

tree rech_inter(char *name, tree t, int ind) {
    if (t == NULL || t->nb_sons == 0) {
        return t;
    }
    if (*t->name != '\0') {
        for (; ind > 0 && name[ind] == t->name[ind]; ind--);
        ind--;
        for (; ind > 0 && name[ind] != '.'; ind--);
        ind++;
    } else {
        for (; ind > 0 && name[ind] != '.'; ind--);
    }
    for (int i = 0; i < t->nb_sons; i++) {
        if (!strcmp(t->sons[i]->name, name + ind)) {
            return rech_inter(name, t->sons[i], ind - 1);
        }
    }
    return t;
}

tree rech(char *name, tree t) {
    return rech_inter(name, t, strlen(name) - 1);
}

void print_tree(tree t) {
    struct server s;
    if (t == NULL) {
        return;
    }
    printf("%s\n", t->name);
    for (int i = 0; i < t->nb_addrs; i++) {
        s = addr_to_string(t->tab_addr[i]); 
        printf("%s:%s ", s.ip, s.port);
    }
    printf("\n");
    for (int i = 0; i < t->nb_sons; i++) {
        print_tree(t->sons[i]);
    }
}

// ajouter un flag "check_sons"
//on peut threader cette partie
char *resolve(int soc, struct request *request, char *dst, tree tree_addr, struct ignored_servers *ignored_serv, bool monitoring, bool free_tab) {    
    if (tree_addr == NULL)
        return dst;
    tree t_rech = rech(request->name, tree_addr);      // on regarde si on a pas déjà fait une requête similaire

    if (t_rech == NULL) {
        return dst;
    }

    if (!strcmp(t_rech->name, request->name)) {                 // si on a déjà trouvé cette adresse, on renvoie directement le résultat
        struct server final_res = addr_to_string(*t_rech->tab_addr);
        snprintf(dst, REQLEN, "%s:%s", final_res.ip, final_res.port);
        return dst;
    }

    if (strcmp(t_rech->name, tree_addr->name) != 0) {           // si on a déjà trouvé des domaines, sous-domaines, etc on reprend de la
        return resolve(soc, request, dst, t_rech, ignored_serv, monitoring, free_tab);
    } else {
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

                                tree son = adj_son(tree_addr, struc_res.name, struc_res.addrs);

                                if (!strcmp(request->name, struc_res.name)) {
                                    struct server final_res = addr_to_string(*struc_res.addrs);
                                    free(struc_res.addrs);
                                    snprintf(dst, REQLEN, "%s:%s", final_res.ip, final_res.port);
                                    return dst;
                                } else {
                                    request->id += 1;
                                    return resolve(soc, request, dst, son, ignored_serv, monitoring, true);
                                }
                            } else {
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
        snprintf(dst, REQLEN, "\33[1;31mTimeout\033[0m");
    }
    return dst;
}

int main(int argc, char const *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: <path of config file>");
        exit(EXIT_FAILURE);
    }

    struct ignored_servers ignored_serv;
    struct request request;
    ignored_serv.nb_servers = 0;

    tree tree_addr = new_tree("\0", parse_conf(argv[1])); 

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
            printf("%s, %s\n", request.name, resolve(soc, &request, res, tree_addr, &ignored_serv, monitoring, false));
            //printf("Noeud racine: %s\n", tree_addr->name);
            print_tree(tree_addr);
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
                printf("%s, %s\n", request.name, resolve(soc, &request, res, tree_addr, &ignored_serv, monitoring, false));
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

    destroy(tree_addr);
    //free(tab_addr);
    //free...
    exit(EXIT_SUCCESS);
}
