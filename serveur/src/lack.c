/**
 * @file lack.c
 * @author Alexandre Vogel, Massimo Venuti
 * @brief Gestion des acquittements - fichier source
 * @date 2020-11-16
 * 
 */
#include "lack.h"

bool addrcmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2) {
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

lack lanew() {
    return NULL;
}

void ladestroy(lack l) {
    if (laempty(l)) {
        return;
    }
    lack n = l->next;
    free(l);
    ladestroy(n);
}

lack laadd(lack l, struct req req, struct sockaddr_in6 addr) {
    lack new;
    MCHK(new = malloc(sizeof(struct s_lack)));
    new->ack.req = req;
    new->ack.addr = addr;
    new->ack.retry = 0;
    PCHK(gettimeofday(&new->ack.time, NULL));
    new->next = lanew();
    lack tmp;
    if (!laempty(l)) {
        for (tmp = l; !laempty(tmp->next); tmp = tmp->next);
        tmp->next = new;
        return l;
    } else {
        return new;
    }
}

lack larm(lack l, int id, struct sockaddr_in6 addr) {
    if (laempty(l)) {
        return lanew();
    }
    if (l->ack.req.id == id && addrcmp(l->ack.addr, addr)) {
        lack n = l->next;
        free(l);
        return n;
    }
    return larm(l->next, id, addr);
}

bool laempty(lack l) { return l == NULL; }

