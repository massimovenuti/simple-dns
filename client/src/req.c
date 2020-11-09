#include "req.h"

struct req new_req(lreq *l, int id, char *name, struct tab_addrs addrs) {
    struct req req;
    req.id = id;
    strcpy(req.name, name);
    req.dest_addrs = addrs;
    req.index = get_index(*l, req);
    *l = lreq_add(*l, req);
    return req;
}

int get_index(lreq l, struct req req) {
    int index = 0;
    lreq tmp;
    for (tmp = l; !lreq_empty(tmp); tmp = tmp->next) {
        if (addr_in(req.dest_addrs.addrs[0], tmp->req.dest_addrs) && tmp->req.index + 1 > index) {
            index = tmp->req.index + 1;
        }
    }
    return index;
}

lreq lreq_new() {
    return NULL;
}

void lreq_destroy(lreq l) {
    if (lreq_empty(l)) {
        return;
    }
    lreq n = l->next;
    free(l);
    lreq_destroy(n);
}

lreq lreq_add(lreq l, struct req x) {
    lreq new;
    MCHK(new = malloc(sizeof(struct s_lreq)));
    new->req = x;
    new->next = lreq_new();
    lreq tmp;
    if (!lreq_empty(l)) {
        for (tmp = l; !lreq_empty(tmp->next); tmp = tmp->next);
        tmp->next = new;
        return l;
    } else {
        return new;
    }
}

lreq lreq_rm(lreq l, int id) {
    if (lreq_empty(l)) {
        return l;
    }
    if (l->req.id == id) {
        lreq n = l->next;
        free(l);
        return n;
    }
    l->next = lreq_rm(l->next, id);
    return l;
}

struct req lreq_elem(lreq l, int i) {
    if (lreq_empty(l)) {
        exit(EXIT_FAILURE);
    }
    if (i == 0) {
        return l->req;
    }
    return lreq_elem(l->next, i - 1);
}

int lreq_len(lreq l) {
    if (lreq_empty(l)) {
        return 0;
    }
    return 1 + lreq_len(l->next);
}

lreq lreq_search(lreq l, int id) {
    if (lreq_empty(l)) {
        return lreq_new();
    }
    if (l->req.id == id) {
        return l;
    }
    return lreq_search(l->next, id);
}

bool lreq_empty(lreq l) { return l == NULL; }
