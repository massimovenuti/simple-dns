#include "parser.h"

void init_names(struct name *names, int start, int end) {
    for (int i = start; i < end; i++) {
        names[i].nb_servers = 0;
        names[i].max_servers = TABSIZE;
        names[i].servers = NULL;
    }
}

void free_names(struct name *names) {
    for (int i = 0; names[i].servers != NULL; i++) {
        free(names[i].servers);
    }
    free(names);
}


int compare(char *s1, char *s2) {
    char *tmp = strstr(s1, s2);
    if (tmp == NULL)
        return 0;
    return tmp[0] == '.' || s1[strlen(s1) - strlen(tmp) - 1] == '.' || !strcmp(s1, s2); 
}

struct name *parse_conf(const char *file_name) {
    struct name *res;
    int max_names = TABSIZE;

    MCHK(res = malloc(max_names * sizeof(struct name)));

    init_names(res, 0, max_names);

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
            init_names(res, i, max_names);
        }
        for (tmp = 0; tmp < i; tmp++) {
            found = !strcmp(res[tmp].name, name);
            if (found) {
                if (res[tmp].nb_servers >= res[tmp].max_servers) {
                    res[tmp].max_servers *= INCREASE_COEF;
                    MCHK(res[tmp].servers = realloc(res[tmp].servers, res[tmp].max_servers * sizeof(struct server)));
                }
                MCHK(strcpy(res[tmp].servers[res[tmp].nb_servers].ip, ip));
                MCHK(strcpy(res[tmp].servers[res[tmp].nb_servers].port, port));
                res[tmp].nb_servers++;
            }
        }

        if (found) {
            i--;
        } else {
            MCHK(strcpy(res[i].name, name));
            MCHK(res[i].servers = malloc(res[i].max_servers * sizeof(struct server)));
            MCHK(strcpy(res[i].servers[0].ip, ip));
            MCHK(strcpy(res[i].servers[0].port, port));
            res[i].nb_servers = 1;
        }
    }
    PCHK(fclose(file));
    return res;
}

int parse_req(char *dest, char *src, size_t len) {
    int i;
    for (i = len; i > 0 && src[i] != '|'; i--)
        ;
    if (i == 0) {
        return EXIT_FAILURE;
    }
    MCHK(strcpy(dest, &src[i + 1]));
    return EXIT_SUCCESS;
}

int increase_memsize(char *dest, size_t *size_dest, size_t size_src) {
    int x = EXIT_FAILURE;
    if (*size_dest < size_src) {
        *size_dest *= INCREASE_COEF;
        MCHK(realloc(dest, *size_dest));
        x = EXIT_SUCCESS;
    }
    return x;
}

int make_res(char *dest, char *src, struct name *names, size_t *len) {
    char name[NAMELEN];
    size_t alloc_mem = sizeof(dest);

    if (!parse_req(name, src, *len)) {
        fprintf(stderr, "Request incorrect\n");
        return EXIT_FAILURE;
    }

    *len += 3;
    increase_memsize(dest, &alloc_mem, *len);

    MCHK(strcat(strcpy(dest, src), SEPARATOR));

    int i, j;
    bool found = false;
    for (i = 0; names[i].nb_servers != 0; i++) {
        if (compare(name, names[i].name)) {
            found = true;
            MCHK(strcat(dest, SUCCESS));
            for (j = 0; j < names[i].nb_servers; j++) {
                *len += 3 + strlen(names[i].name) + strlen(names[i].servers[j].ip) + strlen(names[i].servers[j].port);
                increase_memsize(dest, &alloc_mem, *len);
                MCHK(strcat(dest, SEPARATOR));
                MCHK(strcat(dest, name));
                MCHK(strcat(dest, SUBSEPARATOR));
                MCHK(strcat(dest, names[i].servers[j].ip));
                MCHK(strcat(dest, SUBSEPARATOR));
                MCHK(strcat(dest, names[i].servers[j].port));
            }
        }
    }

    if (!found) {
        MCHK(strcat(dest, FAIL));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}