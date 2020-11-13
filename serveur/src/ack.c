#include "ack.h"

bool addr_cmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2) {
    char ip1[140];
    char ip2[140];
    if (a1.sin6_family != a2.sin6_family) {
        return false;
    }
    if (a1.sin6_family == AF_INET6) {
        inet_ntop(AF_INET6, &a1.sin6_addr, ip1, sizeof(a1));
        inet_ntop(AF_INET6, &a2.sin6_addr, ip2, sizeof(a2));
    } else {
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&a1))->sin_addr, ip1, sizeof(a1));
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&a2))->sin_addr, ip2, sizeof(a2));
    }
    return (a1.sin6_port == a2.sin6_port) && !strcmp(ip1, ip2);
}

lack lack_new() {
    return NULL;
}

void lack_destroy(lack l) {
    if (lack_empty(l)) {
        return;
    }
    lack n = l->next;
    free(l);
    lack_destroy(n);
}

lack lack_add(lack l, int id, struct sockaddr_in6 addr) {
    lack new;
    MCHK(new = malloc(sizeof(struct s_lack)));
    new->ack.id = id;
    new->ack.addr = addr;
    new->next = lack_new();
    lack tmp;
    if (!lack_empty(l)) {
        for (tmp = l; !lack_empty(tmp->next); tmp = tmp->next);
        tmp->next = new;
        return l;
    } else {
        return new;
    }
}

lack lack_rm(lack l, int id, struct sockaddr_in6 addr) {
    if (lack_empty(l)) {
        return lack_new();
    }
    if (l->ack.id == id && addr_cmp(l->ack.addr, addr)) {
        lack n = l->next;
        free(l);
        return n;
    }
    return lack_rm(l->next, id, addr);
}

bool lack_empty(lack l) { return l == NULL; }

