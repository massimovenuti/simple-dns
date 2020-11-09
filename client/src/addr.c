#include "addr.h"

bool addr_cmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2) {
    char ip1[IPLEN];
    char ip2[IPLEN];
    if (a1.sin6_family != a2.sin6_family) {
        return false;
    }
    if (a1.sin6_family == AF_INET6) {
        inet_ntop(AF_INET6, &a1.sin6_addr, ip1, sizeof(a1));
        inet_ntop(AF_INET6, &a2.sin6_addr, ip1, sizeof(a2));
    } else {
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&a1))->sin_addr, ip1, sizeof(a1));
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&a2))->sin_addr, ip1, sizeof(a2));
    }
    return (ntohs(a1.sin6_port) == ntohs(a2.sin6_port)) && !strcmp(ip1, ip2);
}

bool addr_in(struct sockaddr_in6 addr, struct tab_addrs addrs) {
    int i;
    for (i = 0; i < addrs.len && !addr_cmp(addr, addrs.addrs[i]); i++)
        ;
    return i != addrs.len;
}

void fprint_addr(FILE *stream, struct sockaddr_in6 addr) {
    char ip[IPLEN];
    char port[PORTLEN];
    sprintf(port, "%d", ntohs(addr.sin6_port));
    if (addr.sin6_family == AF_INET6) {
        inet_ntop(AF_INET6, &addr.sin6_addr, ip, sizeof(addr));
    } else {
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&addr))->sin_addr, ip, sizeof(addr));
    }
    fprintf(stream, "%s:%s\n", ip, port);
}

laddr laddr_new() {
    return NULL;
}

void laddr_destroy(laddr l) {
    if (laddr_empty(l)) {
        return;
    }
    laddr n = l->next;
    free(l);
    laddr_destroy(n);
}

laddr laddr_add(laddr l, struct sockaddr_in6 x) {
    laddr new_head;
    MCHK(new_head = malloc(sizeof(struct s_laddr)));
    new_head->addr = x;
    new_head->next = l;
    return new_head;
}

laddr laddr_rm(laddr l, struct sockaddr_in6 x) {
    if (laddr_empty(l)) {
        return laddr_new();
    }
    if (addr_cmp(l->addr, x)) {
        laddr n = l->next;
        free(l);
        return n;
    }
    return laddr_rm(l->next, x);
}

struct sockaddr_in6 laddr_elem(laddr l, int i) {
    if (laddr_empty(l)) {
        exit(EXIT_FAILURE);
    }
    if (i == 0) {
        return l->addr;
    }
    return laddr_elem(l->next, i - 1);
}

int laddr_len(laddr l) {
    if (laddr_empty(l)) {
        return 0;
    }
    return 1 + laddr_len(l->next);
}

laddr laddr_search(laddr l, struct sockaddr_in6 x) {
    if (laddr_empty(l)) {
        return laddr_new();
    }
    if (addr_cmp(l->addr, x)) {
        return l;
    }
    return laddr_search(l->next, x);
}

bool laddr_empty(laddr l) { return l == NULL; }

void laddr_fprint(FILE *stream, laddr l) {
    if (laddr_empty(l))
        return;
    fprint_addr(stream, l->addr);
    laddr_fprint(stream, l->next);
}
