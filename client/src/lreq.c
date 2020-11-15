#include "lreq.h"

struct req *new_req(lreq *l, int id, char *name, struct tab_addrs addrs) {
    struct req req;
    req.id = id;
    strcpy(req.name, name);
    req.dest_addrs = addrs;
    req.index = get_index(*l, req);
    return &lradd(l, req)->req;
}

void update_req(lreq *l, struct req *req, int id, struct tab_addrs addrs) {
    req->id = id;
    req->dest_addrs = addrs;
    req->index = get_index(*l, *req);
}

int get_index(lreq l, struct req req) {
    int index = 0;
    lreq tmp;
    for (tmp = l; !lrempty(tmp); tmp = tmp->next) {
        if (belong(req.dest_addrs.addrs[0], tmp->req.dest_addrs) &&
            tmp->req.index + 1 > index) {
            index = tmp->req.index + 1;
        }
    }
    return index;
}

bool addrcmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2) {
    char ip1[LIP];
    char ip2[LIP];
    if (a1.sin6_family != a2.sin6_family) {
        return false;
    }
    if (a1.sin6_family == AF_INET6) {
        inet_ntop(AF_INET6, &a1.sin6_addr, ip1, sizeof(a1));
        inet_ntop(AF_INET6, &a2.sin6_addr, ip2, sizeof(a2));
    } else {
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&a1))->sin_addr, ip1,
                  sizeof(a1));
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&a2))->sin_addr, ip2,
                  sizeof(a2));
    }
    return (a1.sin6_port == a2.sin6_port) && !strcmp(ip1, ip2);
}

bool belong(struct sockaddr_in6 addr, struct tab_addrs addrs) {
    bool found;
    for (int i = 0; i < addrs.len && !found; i++) {
        found = addrcmp(addr, addrs.addrs[i]);
    }
    return found;
}

lreq lrnew() { return NULL; }

void lrfree(lreq l) {
    if (lrempty(l)) {
        return;
    }
    lreq n = l->next;
    free(l);
    lrfree(n);
}

lreq lradd(lreq *l, struct req x) {
    lreq new;
    MCHK(new = malloc(sizeof(struct s_lreq)));
    new->req = x;
    new->next = lrnew();
    lreq tmp;
    if (!lrempty(*l)) {
        for (tmp = *l; !lrempty(tmp->next); tmp = tmp->next)
            ;
        tmp->next = new;
    } else {
        *l = new;
    }
    return new;
}

lreq lrrm(lreq l, int id) {
    if (lrempty(l)) {
        return l;
    }
    if (l->req.id == id) {
        lreq n = l->next;
        free(l);
        return n;
    }
    l->next = lrrm(l->next, id);
    return l;
}

int lrlen(lreq l) {
    if (lrempty(l)) {
        return 0;
    }
    return 1 + lrlen(l->next);
}

lreq lrsearch(lreq l, int id) {
    if (lrempty(l)) {
        return lrnew();
    }
    if (l->req.id == id) {
        return l;
    }
    return lrsearch(l->next, id);
}

bool lrempty(lreq l) { return l == NULL; }
