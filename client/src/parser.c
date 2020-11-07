#include "parser.h"

void convert(char ip[], int port, struct sockaddr_in6 *dst) {
    int check;
    struct sockaddr_in *ipV4addr = (struct sockaddr_in *)(dst);
    PCHK(check = inet_pton(AF_INET, ip, &ipV4addr->sin_addr));

    if (check == 0) {
        PCHK(check = inet_pton(AF_INET6, ip, &dst->sin6_addr));
        if (check == 0) {
            fprintf(stderr, "Invalid address: %s\n", ip);
            exit(EXIT_FAILURE);
        }
        dst->sin6_port = htons(port);
        dst->sin6_family = AF_INET6;
    } else {
        ipV4addr->sin_port = htons(port);
        ipV4addr->sin_family = AF_INET;
    }
}

bool is_ignored(char *ip, char *port, struct ignored_servers servers) {
    int i;
    for (i = 0; i < servers.nb_servers && strcmp(ip, servers.servers[i].ip) != 0 && strcmp(port, servers.servers[i].port) != 0; i++)
        ;
    return i != servers.nb_servers;
}

struct addr_with_flag *parse_conf(const char *file_name) {
    struct addr_with_flag *res;
    int max_addrs = TABSIZE;

    MCHK(res = malloc(max_addrs * sizeof(struct addr_with_flag)));

    FILE *file;

    MCHK(file = fopen(file_name, "r"));

    char ip[IPLEN];
    int port;

    int i;
    for (i = 0; fscanf(file, "%[^|- ] | %d\n", ip, &port) != EOF; i++) {
        if (i >= max_addrs) {
            max_addrs *= INCREASE_COEF;
            MCHK(res = realloc(res, max_addrs * sizeof(struct addr_with_flag)));
        }
        convert(ip, port, &res[i].addr);
        res[i].end = false;
        res[i].ignore = false;
    }
    res[i].end = true;

    PCHK(fclose(file));
    return res;
}

struct res parse_res(char *res, size_t len, struct ignored_servers servers) {
    (void)len;
    struct res s_res;
    char tab_addr[1024];
    *tab_addr = '\0';

    if (sscanf(res, " %d | %ld,%ld | %[^|- ] | %d | %s", &s_res.id, &s_res.time.tv_sec, &s_res.time.tv_usec, s_res.req_name, &s_res.code, tab_addr) < 4) {
        fprintf(stderr, "Incorrect server result\n");
        exit(EXIT_FAILURE);
    }

    struct timeval t;
    PCHK(gettimeofday(&t, NULL));

    s_res.time.tv_sec = t.tv_sec - s_res.time.tv_sec;
    s_res.time.tv_usec = t.tv_usec - s_res.time.tv_usec;

    char *token;
    char name[NAMELEN];
    char ip[IPLEN];
    char port[PORTLEN];
    int nb_addrs = 0;
    int max_addrs = TABSIZE;
    bool first = true;

    token = strtok(tab_addr, SEPARATOR);

    if (token == NULL) {
        s_res.addrs = NULL;
    } else {
        s_res.addrs = malloc(max_addrs * sizeof(struct addr_with_flag));
        do {
            if (sscanf(token, " %[^, ] , %[^, ] , %[^, ] ", name, ip, port) != 3) {
                fprintf(stderr, "Server result incorrect\n");
                exit(EXIT_FAILURE);
            }
            if (first) {
                MCHK(strcpy(s_res.name, name));
                first = false;
            }
            if (nb_addrs > max_addrs) {
                max_addrs *= INCREASE_COEF;
                MCHK(s_res.addrs = realloc(s_res.addrs, max_addrs * sizeof(struct addr_with_flag)));
            }
            if (!is_ignored(ip, port, servers)) {
                convert(ip, atoi(port), &s_res.addrs[nb_addrs].addr);
                s_res.addrs[nb_addrs].ignore = false;
                s_res.addrs[nb_addrs].end = false;
                nb_addrs++;
            }
            token = strtok(NULL, SEPARATOR);
        } while (token != NULL);
        s_res.addrs[nb_addrs].end = true;
    }

    return s_res;
}

struct server addr_to_string(struct addr_with_flag addr) {
    struct server res;
    sprintf(res.port, "%d", ntohs(addr.addr.sin6_port));
    if (addr.addr.sin6_family == AF_INET6) {
        inet_ntop(AF_INET6, &addr.addr.sin6_addr, res.ip, sizeof(addr.addr));
    } else {
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&addr.addr))->sin_addr, res.ip, sizeof(addr.addr));
    }
    return res;
}

struct tree *rech_inter(char *name, struct tree *t, int ind) {
    if (t->nb_sons == 0) {
        return t;
    }
    for (; ind > 0 && name[ind] != '.'; ind--);
    for (int i = 0; i < t->nb_sons; i++) {
        if (!strcmp(t->sons[i].name, name + ind - 1)) {
            return rech_inter(name, &t->sons[i], ind - 1);
        }
    }
    return t;
}

struct tree *rech(char *name, struct tree *t) {
    return rech_inter(name, t, strlen(name) - 1);
}
