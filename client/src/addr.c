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
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&a2))->sin_addr, ip2, sizeof(a2));
    }
    return (a1.sin6_port == a2.sin6_port) && !strcmp(ip1, ip2);
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

void use(struct monitored_addr *a, struct timeval t) {
    struct timeval tmp = op_ntimeval(a->avg_time, '*', a->counter);
    a->avg_time = op_ntimeval(op_timeval(tmp, '+', t), '/', a->counter + 1);
    a->counter++;
    return;
}

struct monitored_addr new_maddr(struct sockaddr_in6 a) {
    struct monitored_addr m_addr;
    m_addr.addr = a;
    m_addr.counter = 0;
    m_addr.avg_time = new_timeval(0, 0);
    return m_addr;
}

laddr laddr_add(laddr l, struct monitored_addr x) {
    laddr new;
    MCHK(new = malloc(sizeof(struct s_laddr)));
    new->m_addr = x;
    new->next = laddr_new();
    laddr tmp;
    if (!laddr_empty(l)) {
        for (tmp = l; !laddr_empty(tmp->next); tmp = tmp->next)
            ;
        tmp->next = new;
        return l;
    } else {
        return new;
    }
}

laddr laddr_rm(laddr l, struct sockaddr_in6 x) {
    if (laddr_empty(l)) {
        return laddr_new();
    }
    if (addr_cmp(l->m_addr.addr, x)) {
        laddr n = l->next;
        free(l);
        return n;
    }
    return laddr_rm(l->next, x);
}

struct monitored_addr laddr_elem(laddr l, int i) {
    if (laddr_empty(l)) {
        exit(EXIT_FAILURE);
    }
    if (i == 0) {
        return l->m_addr;
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
    if (addr_cmp(l->m_addr.addr, x)) {
        return l;
    }
    return laddr_search(l->next, x);
}

bool laddr_empty(laddr l) { return l == NULL; }

void laddr_fprint(FILE *stream, laddr l) {
    if (laddr_empty(l))
        return;
    fprint_addr(stream, l->m_addr.addr);
    laddr_fprint(stream, l->next);
}
