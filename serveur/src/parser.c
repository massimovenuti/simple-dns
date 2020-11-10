#include "parser.h"

struct tab_servers new_tab_servers() {
    struct tab_servers ts;
    ts.servs = NULL;
    ts.len = 0;
    ts.max_len = 0;
    return ts;
}

struct tab_names new_tab_names() {
    struct tab_names tn;
    tn.names = NULL;
    tn.len = 0;
    tn.max_len = 0;
    return tn;
}

struct server new_server() {
    struct server res;
    *res.ip = '\0';
    *res.port = '\0';
    return res;
}

struct name new_name() {
    struct name n;
    *n.name = '\0';
    n.tab_servs = new_tab_servers();
    return n;
}

void free_tab_servers(struct tab_servers *s) {
    free(s->servs);
}

void free_tab_names(struct tab_names *n) {
    for (int i = 0; i < n->len; i++) {
        free_tab_servers(&n->names[i].tab_servs);
    }
    free(n->names);
}

void add_name(struct tab_names *n, char *name, struct tab_servers tab_servs) {
    if (n == NULL) {
        return;
    }
    n->names = inctab(n->names, n->len + 1, &n->max_len, sizeof(struct name), INCREASE_COEF);
    strcpy(n->names[n->len].name, name);
    n->names[n->len].tab_servs = tab_servs;
    n->names[n->len + 1] = new_name();
    n->len++;
}

void add_server(struct tab_servers *s, char *ip, char *port) {
    if (s == NULL) {
        return;
    }
    s->servs = inctab(s->servs, s->len + 1, &s->max_len, sizeof(struct server), INCREASE_COEF);
    MCHK(strcpy(s->servs[s->len].ip, ip));
    MCHK(strcpy(s->servs[s->len].port, port));
    s->servs[s->len + 1] = new_server();
    s->len++;
}

int search_name(struct tab_names n, char *name) {
    for (int tmp = 0; tmp < n.len; tmp++) {
        if (compare(name, n.names[tmp].name)) {
            return tmp;
        }
    }
    return -1;
}

// augmente la taille d'un tableau quelconque en fonction de len
void *inctab(void *dest, int len, int *max_len, size_t size_elem, int coef) {
    if (len >= *max_len) {
        if (*max_len == 0) {
            *max_len = TABSIZE + len;
        } else {
            *max_len *= coef;
        }
        MCHK(dest = realloc(dest, *max_len * size_elem * coef));
    }
    return dest;
}

char *incstr(char *dest, size_t len, size_t *max_len, int coef) {
    if (len >= *max_len) {
        if (*max_len == 0) {
            *max_len = (TABSIZE * sizeof(char)) + len;
        } else {
            *max_len *= coef;
        }
        MCHK(dest = realloc(dest, *max_len * coef));
    }
    return dest;
}

bool compare(char *s1, char *s2) {
    char *tmp = strstr(s1, s2);
    if (tmp == NULL)
        return false;
    return tmp[0] == '.' || s1[strlen(s1) - strlen(tmp) - 1] == '.' || !strcmp(s1, s2);
}

struct tab_names parse_conf(const char *file_name) {
    struct tab_names n = new_tab_names();
    FILE *file;
    MCHK(file = fopen(file_name, "r"));

    char name[NAMELEN];
    char ip[IPLEN];
    char port[PORTLEN];
    int code;
    while ((code = fscanf(file, "%[^|- ] | %140[^|- ] | %10s\n", name, ip, port)) == 3 && code != EOF) {
        int index = search_name(n, name);
        if (index >= 0) {
            add_server(&n.names[index].tab_servs, ip, port);
        } else {
            struct tab_servers new_ts = new_tab_servers();
            add_server(&new_ts, ip, port);
            add_name(&n, name, new_ts);
        }
    }

    if (code != EOF) {
        fprintf(stderr, "Bad conf\n");
        exit(EXIT_FAILURE);
    }
    PCHK(fclose(file));
    return n;
}

bool parse_req(char *dest, char *src) {
    if (sscanf(src, " %*[^| ] | %*[^| ] | %[^| ] ", dest)) { /* /!\ longueur des chaînes */
        return true;
    }
    return false;
}

bool make_res(char *dest, char *src, struct tab_names n, size_t *new_len_dest, size_t len_src, size_t *max_len_dest) {
    char name[NAMELEN];

    if (!parse_req(name, src)) {
        fprintf(stderr, "Request incorrect\n");
        return false;
    }

    *new_len_dest = len_src + 2; /* /!\ */
    dest = incstr(dest, *new_len_dest, max_len_dest, INCREASE_COEF);
    MCHK(strcat(strcpy(dest, src), SEPARATOR));

    int ind = search_name(n, name);
    if (ind >= 0) {
        MCHK(strcat(dest, SUCCESS));
        for (int j = 0; j < n.names[ind].tab_servs.len; j++) {
            *new_len_dest += 3 + strlen(n.names[ind].name) + strlen(n.names[ind].tab_servs.servs[j].ip) + strlen(n.names[ind].tab_servs.servs[j].port);
            dest = incstr(dest, *max_len_dest, max_len_dest, INCREASE_COEF);
            MCHK(strcat(dest, SEPARATOR));
            MCHK(strcat(dest, n.names[ind].name));
            MCHK(strcat(dest, SUBSEPARATOR));
            MCHK(strcat(dest, n.names[ind].tab_servs.servs[j].ip));
            MCHK(strcat(dest, SUBSEPARATOR));
            MCHK(strcat(dest, n.names[ind].tab_servs.servs[j].port));
        }
    } else {
        *new_len_dest += 2; /* /!\ */
        MCHK(strcat(dest, FAIL));
        MCHK(strcat(dest, SEPARATOR));
    }
    
    MCHK(strcat(dest, "\0"));

    return true;
}
