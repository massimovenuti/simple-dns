#include "parser.h"

int compare(char *s1, char *s2) {
    char *tmp = strstr(s1, s2);
    if (tmp == NULL)
        return 0;
    return tmp[0] == '.' || s1[strlen(s1) - strlen(tmp) - 1] == '.'; // erreur 
}

struct name *parse_conf(const char *file_name) {
    struct name *res;
    int max_names = TABSIZE;

    MCHK(res = malloc(max_names * sizeof(struct name)));

    for (int i = 0; i < max_names; i++) {
        res[i].nb_servers = 0;
        res[i].max_servers = TABSIZE;
        res[i].servers = NULL;
    }

    FILE *file;

    MCHK(file = fopen(file_name, "r"));

    char name[NAMELEN];
    char ip[IPLEN];
    char port[PORTLEN];

    int i, tmp;
    bool found;
    for (i = 0; fscanf(file, "%[^|- ] | %[^|- ] | %s\n", name, ip, port) != EOF; i++) {
        found = false;
        if (i >= max_names) {
            max_names *= INCREASE_COEF;
            MCHK(res = realloc(res, max_names * sizeof(struct name)));
            for (int j = i; j < max_names; j++) {
                res[j].nb_servers = 0;
                res[j].max_servers = TABSIZE;
                res[j].servers = NULL;
            }
        }
        for (tmp = 0; tmp < i; tmp++) {
            found = strcmp(res[tmp].name, name);
            if (found) {
                if (res[tmp].nb_servers == 0) {
                    res[tmp].servers = malloc(res[tmp].max_servers * sizeof(struct server));
                } 
                if (res[tmp].nb_servers >= res[tmp].max_servers) {
                    res[tmp].max_servers *= INCREASE_COEF;
                    res[tmp].servers = realloc(res[tmp].servers, res[tmp].max_servers * sizeof(struct server));
                }
                MCHK(strcpy(res[tmp].servers[res[tmp].nb_servers].ip, ip));
                MCHK(strcpy(res[tmp].servers[res[tmp].nb_servers].port, port));
                res[tmp].nb_servers++;
            }
        }

        if (!found) {
            MCHK(strcpy(res[i].name, name));
            MCHK(strcpy(res[i].servers[0].ip, ip));
            MCHK(strcpy(res[i].servers[0].port, port));
            res[i].nb_servers = 1;
        } else {
            i--;
        }
    }
    PCHK(fclose(file));
    return res;
}

char *parse_req(char *str, size_t len) {
    char *res;
    int i;
    for (i = len; i > 0 && str[i] != '|'; i--)
        ;
    if (i == 0) {
        fprintf(stderr, "Request incorrect\n");
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
    bool found = false;
    for (i = 0; names[i].nb_servers != 0; i++) {
        if (compare(req, names[i].name)) {
            found = true;
            MCHK(strcat(res, SUCCESS));
            for (j = 0; j < names[i].nb_servers; j++) {
                *len += 3 + strlen(names[i].name) + strlen(names[i].servers[j].ip) + strlen(names[i].servers[j].port);
                MCHK(realloc(res, *len));
                MCHK(strcat(res, SEPARATOR));
                MCHK(strcat(res, names[i].name));
                MCHK(strcat(res, SUBSEPARATOR));
                MCHK(strcat(res, names[i].servers[j].ip));
                MCHK(strcat(res, SUBSEPARATOR));
                MCHK(strcat(res, names[i].servers[j].port));
            }
        }
    }

    if (!found)
        MCHK(strcat(res, FAIL));
    return res;
}