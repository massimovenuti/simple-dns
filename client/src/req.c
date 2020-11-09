#include "req.h"

struct req new_req(lreq l, int id, char *name, struct tab_addrs addrs) {
    struct req req;
    struct timeval t;
    t.tv_sec = 1;   // à modifier
    t.tv_usec = 0;  // à modifier
    req.id = id;
    strcpy(req.name, name);
    req.dest_addrs = addrs;
    req.t = t;
    req.index = get_index(l, req);
    l = lreq_add(l, req);
    return req;
}

int get_index(lreq l, struct req req) {
    int index = 0;
    lreq tmp;
    for (tmp = l; !lreq_empty(l); tmp = tmp->next) {
        if (addr_in(req.dest_addrs.addrs[0], tmp->req.dest_addrs) && tmp->req.index > index) {
            index = tmp->req.index;
        }
    }
    return index + 1;
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
    for (tmp = l; !lreq_empty(tmp->next); tmp = tmp->next);
    tmp->next = new;
    return l;
}

lreq lreq_rm(lreq l, int id) {
    if (lreq_empty(l)) {
        return lreq_new();
    }
    if (l->req.id == id) {
        lreq n = l->next;
        free(l);
        return n;
    }
    return lreq_rm(l->next, id);
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
