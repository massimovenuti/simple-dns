#include "parser.h"

void convert(char ip[], int port, struct sockaddr_in6 *dst) {
    int check;
    struct sockaddr_in *ipV4addr = (struct sockaddr_in *)(dst);
    PCHK(check = inet_pton(AF_INET, ip, &ipV4addr->sin_addr));

    if (check == 0) {
        PCHK(check = inet_pton(AF_INET6, ip, &dst->sin6_addr));
        if (check == 0) {
            fprintf(stderr, "invalide addrese: %s\n", ip);
            exit(EXIT_FAILURE);
        }
        dst->sin6_port = htons(port);
        dst->sin6_family = AF_INET6;
    } else {
        ipV4addr->sin_port = htons(port);
        ipV4addr->sin_family = AF_INET;
    }
}

struct addr_with_flag *parse_conf(const char *file_name) {
    struct addr_with_flag *res;
    int max_addrs = TABSIZE;

    MCHK(res = malloc(max_addrs * sizeof(struct addr_with_flag)));

    FILE *file;

    MCHK(file = fopen(file_name, "r"));

    char ip[100];
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
    res[i + 1].end = true;

    PCHK(fclose(file));
    return res;
}

struct res parse_res(char *res, size_t len) {
    (void)len;
    struct res s_res;
    char id[IDLEN];
    char time[TIMELEN];
    char code[CODELEN];

    sscanf(res, "%[^|- ] | %[^|- ] | %[^|- ] | %[^|- ]", id, time, s_res.req_name, code);

    s_res.id = atoi(id);
    s_res.time = (time_t)atoi(time);
    s_res.code = atoi(code);
    
    len -= (strlen(id) + strlen(time) + strlen(s_res.req_name) + strlen(code));

    char *token;
    char name[NAMELEN];
    char ip[IPLEN];
    char port[PORTLEN];
    int nb_addrs = 0;
    int max_addrs = TABSIZE;
    bool first = true;

    token = strtok(res + len, "|");

    if (token == NULL) {
        s_res.addrs = NULL;
    } else {
        s_res.addrs = malloc(max_addrs * sizeof(struct addr_with_flag));
        do {
            if (nb_addrs > max_addrs) {
                max_addrs *= INCREASE_COEF;
                s_res.addrs = realloc(s_res.addrs, max_addrs * sizeof(struct addr_with_flag));
            }
            sscanf(token, " %[^,- ] , %[^,- ] , %[^,- ] ", name, ip, port);
            if (first) {
                strcpy(s_res.name, name);
                first = false;
            }
            if (*name && *ip && *port) {
                convert(ip, atoi(port), &s_res.addrs[nb_addrs].addr);
                s_res.addrs[nb_addrs].ignore = false;
                s_res.addrs[nb_addrs].end = false;
                nb_addrs++;
            }
            token = strtok(NULL, "|");
        } while (token != NULL);
        s_res.addrs[nb_addrs].end = true;
    }

    return s_res;
}
