/**
 * @file parser.c
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Parser côté serveur - fichier source
 * @date 2020-11-16
 * 
 */

#include "parser.h"

/**
 * @brief Initialise un tableau de struct name
 * 
 * @param names Tableau de struct name - doit être alloué
 * @param start Inidice du premier élément du tableau à initialiser
 * @param end Indicie du dernier élément du tableau à intialiser
 */
void init_names(struct name *names, int start, int end) {
    for (int i = start; i < end; i++) {
        names[i].nb_servers = 0;
        names[i].max_servers = TABSIZE;
        names[i].servers = NULL;
    }
}

/**
 * @brief Libère un tableau de struct name
 * 
 * @param names Tableau de struct name - doit être alloué
 */
void free_names(struct name *names) {
    for (int i = 0; names[i].servers != NULL; i++) {
        free(names[i].servers);
    }
    free(names);
}

/**
 * @brief Compare deux chaînes de caractères
 * 
 * Test si deux chaînes de caractères sont identiques sans prendre en compte un
 * éventuel caractère "." devant la seconde chaîne.
 * 
 * Exemple: compare("fr", ".fr") renvoie vrai
 * 
 * @param s1 Première chaîne de caractères à comparer
 * @param s2 Deuxième chaîne de caractères à comparer
 * @return true Si les chaînes sont identiques à un éventuel caractère "." près
 * @return false Sinon
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
 * @return struct name* Liste des noms avec adresses
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

/**
 * @brief Parse une requête
 * 
 * Extrait la partie "nom" d'une requête.
 * 
 * @param dest Chaîne qui va contenir le résultat - doit être allouée
 * @param src Chaîne représentant la requête à parser sous le format <id>|<time>|<nom>
 * @return true Si succès
 * @return false Sinon
 */
bool parse_req(char *dest, char *src) {
    if (sscanf(src, " %*[^| ] | %*[^| ] | %[^| ] ", dest)) {
        return true;
    }
    return false;
}

/**
 * @brief Augmente la taille mémoire d'une chaîne si elle est trop petite
 * 
 * Augmente la taille mémoire d'une chaîne de caractères avec une taille 
 * souhaitée. Si la taille souhaitée est plus petite que la taille actuelle de
 * la chaîne, rien n'est modifié.
 * 
 * @param dest Chaîne de caractères à tester
 * @param size_dest Taille actuelle de dest
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
 * @brief Construit la réponse à une requête
 * 
 * @param dest Chaîne qui va contenir le résultat - doit être allouée
 * @param src Chaîne représentant la requête sous le format <id>|<time>|<nom>
 * @param names Liste des noms avec adresses propres au serveur
 * @param len_dest Longueur de la chaîne dest - modifiée par la fonction
 * @param len_src Longueur de la chaîne src
 * @param max_len_dest Taille maximale de dest - modifiée par la fonction
 * @return true Si la requête aboutit à un succès
 * @return false Sinon
 */
bool make_res(char *dest, char *src, struct name *names, size_t *len_dest, size_t len_src, size_t *max_len_dest) {
    char name[NAMELEN];

    if (!parse_req(name, src)) {
        fprintf(stderr, "Request incorrect\n");
        exit(EXIT_FAILURE);
    }

    *len_dest = len_src + 4;
    increase_memsize(dest, max_len_dest, *len_dest * sizeof(char));
    MCHK(strcat(strcpy(dest, src), SEPARATOR));

    int i, j;
    bool found = false;
    for (i = 0; names[i].nb_servers != 0; i++) {
        if (compare(name, names[i].name)) {
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

    MCHK(strcat(dest, "\0"));
    if (!found) {
        *len_dest += 1;
        MCHK(strcat(dest, FAIL));
        MCHK(strcat(dest, SEPARATOR));
        return false;
    }

    return true;
}