/**
 * @file parser.c
 * @author Alexandre Vogel, Massimo Venuti
 * @brief Parser côté serveur - fichier source
 * @date 2020-11-16
 *
 */

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

struct server new_server(char *ip, char *port) {
    struct server res;
    strcpy(res.ip, ip);
    strcpy(res.port, port);
    return res;
}

struct name new_name(char *name, struct tab_servers servs) {
    struct name res;
    strcpy(res.name, name);
    res.tab_servs = servs;
    return res;
}

void free_tab_servers(struct tab_servers *s) { free(s->servs); }

void free_tab_names(struct tab_names *n) {
    for (int i = 0; i < n->len; i++) {
        free_tab_servers(&n->names[i].tab_servs);
    }
    free(n->names);
}

void add_name(struct tab_names *tn, struct name n) {
    if (tn == NULL) {
        return;
    }
    tn->names = inctab(tn->names, tn->len + 1, &tn->max_len,
                       sizeof(struct name), INCREASE_COEF);
    tn->names[tn->len] = n;
    tn->names[tn->len + 1] = new_name("\0", new_tab_servers());
    tn->len += 1;
}

void add_server(struct tab_servers *ts, struct server s) {
    if (ts == NULL) {
        return;
    }
    ts->servs = inctab(ts->servs, ts->len + 1, &ts->max_len,
                       sizeof(struct server), INCREASE_COEF);
    ts->servs[ts->len] = s;
    ts->servs[ts->len + 1] = new_server("\0", "\0");
    ts->len += 1;
}

int search_name(struct tab_names n, char *name) {
    for (int tmp = 0; tmp < n.len; tmp++) {
        if (compare(name, n.names[tmp].name)) {
            return tmp;
        }
    }
    return -1;
}

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
    if (tmp == NULL) return false;
    return tmp[0] == '.' || s1[strlen(s1) - strlen(tmp) - 1] == '.' ||
           !strcmp(s1, s2);
}

struct tab_names parse_conf(const char *file_name) {
    struct tab_names n = new_tab_names();
    FILE *file;
    MCHK(file = fopen(file_name, "r"));

    char name[NAMELEN];
    char ip[IPLEN];
    char port[PORTLEN];
    int code;
    while ((code = fscanf(file, "%[^|- ] | %140[^|- ] | %10s\n", name, ip,
                          port)) == 3 &&
           code != EOF) {
        int index = search_name(n, name);
        if (index >= 0) {
            add_server(&n.names[index].tab_servs, new_server(ip, port));
        } else {
            struct tab_servers new_ts = new_tab_servers();
            add_server(&new_ts, new_server(ip, port));
            add_name(&n, new_name(name, new_ts));
        }
    }

    if (code != EOF) {
        fprintf(stderr, "Bad conf\n");
        exit(EXIT_FAILURE);
    }
    PCHK(fclose(file));
    return n;
}

struct req parse_req(char *src) {
    struct req res;
    if (sscanf(src, " %d | %ld,%ld | %[^|- ]", &res.id, &res.time.tv_sec,
               &res.time.tv_usec, res.name) != 4) {
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

char *strcat_res(char *dest, size_t *len_dest, size_t *max_len_dest, char *code,
                 char *name, char *ip, char *port) {
    size_t size = sizeof(char);
    if (code != NULL) {
        if (*code == '1') {
            *len_dest += 2;
            dest = incstr(dest, *len_dest, max_len_dest, INCREASE_COEF);
            MCHK(strcat(dest, SEPARATOR));
            MCHK(strcat(dest, SUCCESS));
        } else {
            *len_dest += 4;
            dest = incstr(dest, *len_dest, max_len_dest, INCREASE_COEF);
            MCHK(strcat(dest, SEPARATOR));
            MCHK(strcat(dest, FAIL));
            MCHK(strcat(dest, SEPARATOR));
        }
        return dest;
    }
    *len_dest += (3 * size) + strlen(name) + strlen(ip) + strlen(port);
    dest = incstr(dest, *len_dest, max_len_dest, INCREASE_COEF);
    MCHK(strcat(dest, SEPARATOR));
    MCHK(strcat(dest, name));
    MCHK(strcat(dest, SUBSEPARATOR));
    MCHK(strcat(dest, ip));
    MCHK(strcat(dest, SUBSEPARATOR));
    MCHK(strcat(dest, port));
    return dest;
}
char *make_res(char *dest, struct req req, struct tab_names n, size_t *len_dest,
               size_t len_src, size_t *max_len_dest) {
    incstr(dest, len_src, max_len_dest, INCREASE_COEF);

    if ((*len_dest = snprintf(dest, *max_len_dest, "%d|%ld,%ld|%s", req.id,
                              req.time.tv_sec, req.time.tv_usec, req.name)) >
        *max_len_dest - 1) {
        fprintf(stderr, "Request too long");
    }

    int ind = search_name(n, req.name);
    if (ind >= 0) {
        dest =
            strcat_res(dest, len_dest, max_len_dest, SUCCESS, NULL, NULL, NULL);
        for (int j = 0; j < n.names[ind].tab_servs.len; j++) {
            dest = strcat_res(dest, len_dest, max_len_dest, NULL,
                              n.names[ind].name,
                              n.names[ind].tab_servs.servs[j].ip,
                              n.names[ind].tab_servs.servs[j].port);
        }
    } else {
        dest = strcat_res(dest, len_dest, max_len_dest, FAIL, NULL, NULL, NULL);
    }

    *len_dest += 1;
    MCHK(strcat(dest, "\0"));

    return dest;
}
