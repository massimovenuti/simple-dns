#include "parser.h"

void convert(char ip[], int port, struct sockaddr *dst) {
    int check;
    struct sockaddr_in *ipV4addr = (struct sockaddr_in *)(dst);
    PCHK(check = inet_pton(AF_INET, ip, &ipV4addr->sin_addr));

    if (check == 0) {
        struct sockaddr_in6 *ipV6addr = (struct sockaddr_in6 *)(dst);
        PCHK(check = inet_pton(AF_INET6, ip, &ipV6addr->sin6_addr));
        if (check == 0) {
            fprintf(stderr, "invalide addrese: %s\n", ip);
            exit(EXIT_FAILURE);
        }
        ipV6addr->sin6_port = htons(port);
        ipV6addr->sin6_family = AF_INET6;
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
    res[i + 1].end = true;

    PCHK(fclose(file));
    return res;
}

struct res parse_res(char *res, size_t len) {
    struct res s_res;
    char id[IDLEN];
    char time[TIMELEN];
    char code[CODELEN];

    sscanf(res, "%[^|- ] | %[^|- ] | %[^|- ] | %[^|- ]", id, time, s_res.req_name, code);

    s_res.id = atoi(id);
    s_res.time = (time_t)atoi(time);
    s_res.code = atoi(code);

    char *token;
    char name[NAMELEN];
    char ip[IPLEN];
    char port[PORTLEN];
    int nb_addrs = 0;
    int max_addrs = TABSIZE;
    bool first = true;

    len -= (strlen(id) + strlen(time) + strlen(s_res.req_name) + strlen(code));
    token = strtok(res + len, SEPARATOR);

    if (token == NULL) {
        s_res.addrs = NULL;
    } else {
        s_res.addrs = malloc(max_addrs * sizeof(struct addr_with_flag));
        do {
            if (nb_addrs > max_addrs) {
                max_addrs *= INCREASE_COEF;
                MCHK(s_res.addrs = realloc(s_res.addrs, max_addrs * sizeof(struct addr_with_flag)));
            }
            sscanf(token, " %[^,- ] , %[^,- ] , %[^,- ] ", name, ip, port);
            if (first) {
                MCHK(strcpy(s_res.name, name));
                first = false;
            }
            if (*name && *ip && *port) {
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
