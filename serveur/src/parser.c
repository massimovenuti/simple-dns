#include "parser.h"

/**
 * @brief 
 * 
 * @param names 
 * @param start 
 * @param end 
 */
void init_names(struct name *names, int start, int end) {
    for (int i = start; i < end; i++) {
        names[i].nb_servers = 0;
        names[i].max_servers = TABSIZE;
        names[i].servers = NULL;
    }
}

/**
 * @brief 
 * 
 * @param names 
 */
void free_names(struct name *names) {
    for (int i = 0; names[i].servers != NULL; i++) {
        free(names[i].servers);
    }
    free(names);
}

/**
 * @brief 
 * 
 * @param s1 
 * @param s2 
 * @return true 
 * @return false 
 */
bool compare(char *s1, char *s2) {
    char *tmp = strstr(s1, s2);
    if (tmp == NULL)
        return false;
    return tmp[0] == '.' || s1[strlen(s1) - strlen(tmp) - 1] == '.' || !strcmp(s1, s2);
}

/**
 * @brief Parse un fichier de configuration
 * 
 * Récupère la liste des noms et des adresses contenues dans un fichier de 
 * configuration.
 * 
 * @param file_name Fichier de configuration
 * @return struct name* 
 */
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

    int i, tmp, code;
    bool found;
    for (i = 0; (code = fscanf(file, "%[^|- ] | %140[^|- ] | %10s\n", name, ip, port)) == 3 && code != EOF; i++) {
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
    if (code != EOF) {
        fprintf(stderr, "Bad conf\n");
        exit(EXIT_FAILURE);
    }

    PCHK(fclose(file));
    return res;
}

/**
 * @brief Parse une requête
 * 
 * Extrait le la partie "nom" d'une requête.
 * 
 * @param dest Chaîne qui va contenir le résultat - doit être allouée
 * @param src Chaîne représentant la requête à parser
 * @return true Si succès
 * @return false Sinon
 */
struct req parse_req(char *src) {
    struct req res;
    if (sscanf(src, " %d | %ld,%ld | %[^|- ]", &res.id, &res.time.tv_sec, &res.time.tv_usec, res.name) != 4) {
        res.id = -1;
    }
    return res;
}

int is_ack(char str[]) {
    int res;
    if (sscanf(str, "ack | %d", &res)) {
        return res;
    }
    return -1;
}

/**
 * @brief Augmente la taille mémoire d'une chaîne si elle est trop petite
 * 
 * Augmente la taille mémoire d'une chaîne de caractères avec une taille 
 * souhaitée. Si la taille souhaitée est plus petite que la taille actuelle de
 * la chaîne, rien n'est modifié.
 * 
 * @param dest Chaîne de caractères à tester
 * @param size_dest Taille actuelle de la chaîne de caractères
 * @param size_src Nouvelle taille souhaitée
 * @return true Si la taille mémoire de dest a été augmentée
 * @return false Sinon
 */
bool increase_memsize(char *dest, size_t *size_dest, size_t size_src) {
    int x = false;
    if (*size_dest < size_src) {
        *size_dest *= INCREASE_COEF;
        MCHK(dest = realloc(dest, *size_dest));
        x = true;
    }
    return x;
}

/**
 * @brief 
 * 
 * @param dest 
 * @param src 
 * @param names 
 * @param len_dest 
 * @param len_src 
 * @param max_len_dest 
 * @return true 
 * @return false 
 */
char * make_res(char *dest, struct req req, struct name *names, size_t *len_dest, size_t len_src, size_t *max_len_dest) {
    increase_memsize(dest, max_len_dest, len_src * sizeof(char));

    if ((*len_dest = snprintf(dest, *max_len_dest, "%d|%ld,%ld|%s", req.id, req.time.tv_sec, req.time.tv_usec, req.name)) > *max_len_dest - 1) {
        fprintf(stderr, "Request too long");
    }
    *len_dest += 2;
    increase_memsize(dest, max_len_dest, *len_dest * sizeof(char));
    MCHK(strcat(dest, SEPARATOR));

    int i, j;
    bool found = false;
    for (i = 0; names[i].nb_servers != 0; i++) {
        if (compare(req.name, names[i].name)) {
            found = true;
            MCHK(strcat(dest, SUCCESS));
            for (j = 0; j < names[i].nb_servers; j++) {
                *len_dest += 3 + strlen(names[i].name) + strlen(names[i].servers[j].ip) + strlen(names[i].servers[j].port);
                increase_memsize(dest, max_len_dest, *len_dest * sizeof(char));
                MCHK(strcat(dest, SEPARATOR));
                MCHK(strcat(dest, names[i].name));
                MCHK(strcat(dest, SUBSEPARATOR));
                MCHK(strcat(dest, names[i].servers[j].ip));
                MCHK(strcat(dest, SUBSEPARATOR));
                MCHK(strcat(dest, names[i].servers[j].port));
            }
        }
    }

    if (!found) {
        *len_dest += 2;
        MCHK(strcat(dest, FAIL));
        MCHK(strcat(dest, SEPARATOR));
    }

    *len_dest += 1;
    MCHK(strcat(dest, "\0"));
    return dest;
}
