#include "lserv.h"

struct server new_server(struct sockaddr_in6 a) {
    struct timeval t = new_timeval(0, 0);
    struct server serv = {a, 0, 0, t};
    return serv;
}

void add_shipment(struct server *s) { s->nb_shipments++; }

void add_reply(struct server *s, struct timeval t) {
    struct timeval tmp = op_ntimeval(s->avg_reply_time, s->nb_replies, '*');
    s->avg_reply_time =
        op_ntimeval(op_timeval(tmp, t, '+'), s->nb_replies + 1, '/');
    s->nb_replies++;
    return;
}

void afprint(FILE *stream, struct sockaddr_in6 addr) {
    char ip[IPLEN];
    char port[PORTLEN];
    sprintf(port, "%d", ntohs(addr.sin6_port));
    if (addr.sin6_family == AF_INET6) {
        inet_ntop(AF_INET6, &addr.sin6_addr, ip, sizeof(addr));
    } else {
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&addr))->sin_addr, ip,
                  sizeof(addr));
    }
    fprintf(stream, "%s:%s", ip, port);
}

void sfprint(FILE *stream, struct server serv) {
    afprint(stream, serv.addr);
    fprintf(stream, NEWLINE);
    if (serv.nb_shipments > 0) {
        fprintf(stream, "number of shipments  %d\n", serv.nb_shipments);
    }
    if (serv.nb_replies > 0) {
        fprintf(stream, "number of replies    %d\n", serv.nb_replies);
    }
    if (serv.avg_reply_time.tv_sec + serv.avg_reply_time.tv_usec > 0) {
        fprintf(stream, "average reply time   %fs" NEWLINE NEWLINE,
                get_timevalue(serv.avg_reply_time));
    }
}

lserv lsnew() { return NULL; }

void lsfree(lserv l) {
    if (lsempty(l)) {
        return;
    }
    lserv n = l->next;
    free(l);
    lsfree(n);
}

lserv lsadd(lserv l, struct server x) {
    lserv new;
    MCHK(new = malloc(sizeof(struct s_lserv)));
    new->server = x;
    new->next = lsnew();
    lserv tmp;
    if (!lsempty(l)) {
        for (tmp = l; !lsempty(tmp->next); tmp = tmp->next)
            ;
        tmp->next = new;
        return l;
    } else {
        return new;
    }
}

lserv lsrm(lserv l, struct sockaddr_in6 x) {
    if (lsempty(l)) {
        return lsnew();
    }
    if (addrcmp(l->server.addr, x)) {
        lserv n = l->next;
        free(l);
        return n;
    }
    return lsrm(l->next, x);
}

lserv lsrmh(lserv l) {
    if (lsempty(l)) {
        return lsnew();
    }
    lserv n = l->next;
    free(l);
    return n;
}

bool lsbelong(struct sockaddr_in6 addr, lserv servs) {
    return !lsempty(lssearch(servs, addr));
}

int lslen(lserv l) {
    if (lsempty(l)) {
        return 0;
    }
    return 1 + lslen(l->next);
}

lserv lssearch(lserv l, struct sockaddr_in6 x) {
    if (lsempty(l)) {
        return lsnew();
    }
    if (addrcmp(l->server.addr, x)) {
        return l;
    }
    return lssearch(l->next, x);
}

bool lsempty(lserv l) { return l == NULL; }

void lsfprint(FILE *stream, lserv l) {
    if (lsempty(l)) {
        return;
    }
    sfprint(stream, l->server);
    lsfprint(stream, l->next);
}
