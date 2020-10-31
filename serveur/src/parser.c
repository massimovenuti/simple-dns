#include "parser.h"

int compare(char *s1, char *s2) {
    char *tmp = strstr(s1, s2);
    if (tmp == NULL)
        return 0;
    return tmp[0] == '.' || s1[strlen(s1) - strlen(tmp) - 1] == '.';
}

struct name *parse_conf(const char *file_name) {
    struct name *res;
    size_t alloc_mem;

    alloc_mem = TABSIZE * sizeof(struct name);
    MCHK(res = malloc(alloc_mem));

    FILE *file;

    MCHK(file = fopen(file_name, "r"));

    char name[NAMELEN];
    char ip[IPLEN];
    char port[PORTLEN];

    size_t i, tmp;
    int find;
    for (i = 0; fscanf(file, "%[^|- ] | %[^|- ] | %s\n", name, ip, port) != EOF; i++) {
        find = 0;
        if (i >= alloc_mem) {
            alloc_mem *= 3;
            MCHK(res = realloc(res, alloc_mem));
        }

        for (tmp = 0; tmp < i; tmp++) {
            if (!strcmp(res[tmp].name, name)) {
                find = 1;
                MCHK(strcpy(res[tmp].servers[res[tmp].nbserv].ip, ip));
                MCHK(strcpy(res[tmp].servers[res[tmp].nbserv].port, port));
                res[tmp].nbserv++;
            }
        }

        if (!find) {
            MCHK(strcpy(res[i].name, name));
            MCHK(strcpy(res[i].servers[0].ip, ip));
            MCHK(strcpy(res[i].servers[0].port, port));
            res[i].nbserv = 1;
        } else {
            i--;
        }
    }
    res[i + 1].nbserv = 0;
    PCHK(fclose(file));
    return res;
}

char *parse_req(char *str, size_t len) {
    char *res;
    int i;
    for (i = len; i > 0 && str[i] != '|'; i--)
        ;
    if (i == 0) {
        fprintf(stderr, "Requête incorrecte\n");
        exit(EXIT_FAILURE);
    }
    MCHK(res = malloc((len - i - 1) * sizeof(char)));
    MCHK(strcpy(res, &str[i + 1]));
    return res;
}

//vérifier l'allocation mémoire
char *make_res(struct name *names, char *req, size_t *len) {
    char *res;
    *len += 3;
    MCHK(res = malloc(*len));
    MCHK(strcpy(res, req));
    MCHK(strcat(res, SEPARATOR));

    int i, j;
    int find = 0;
    for (i = 0; names[i].nbserv != 0; i++) {
        if (compare(req, names[i].name)) {
            find = 1;
            MCHK(strcat(res, SUCCESS));
            for (j = 0; j < names[i].nbserv; j++) {
                *len += 3 + strlen(names[i].name) + strlen(names[i].servers[j].ip) + strlen(names[i].servers[j].port);
                MCHK(realloc(res, *len));
                MCHK(strcat(res, SEPARATOR));
                MCHK(strcat(res, names[i].name));
                MCHK(strcat(res, ","));
                MCHK(strcat(res, names[i].servers[j].ip));
                MCHK(strcat(res, ","));
                MCHK(strcat(res, names[i].servers[j].port));
            }
        }
    }

    if (!find)
        MCHK(strcat(res, FAIL));
    return res;
}