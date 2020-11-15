#ifndef __ADDR_H__
#define __ADDR_H__

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "macro.h"
#include "lreq.h"
#include "time.h"

struct server {
    struct sockaddr_in6 addr;
    int nb_replies;
    int nb_shipments;
    struct timeval avg_reply_time;
};

typedef struct s_lserv {
    struct server server;
    struct s_lserv *next;
} * lserv;

void add_shipment(struct server *s);

void add_reply(struct server *s, struct timeval t);

struct server new_server(struct sockaddr_in6 a);

void afprint(FILE *stream, struct sockaddr_in6 addr);

void sfprint(FILE *stream, struct server serv); /* /!\ à compléter */

lserv lsnew();

void lsfree(lserv l);

lserv lsadd(lserv l, struct server x);

lserv lsrm(lserv l, struct sockaddr_in6 x);

lserv lsrmh(lserv l);

int lslen(lserv l);

lserv lssearch(lserv l, struct sockaddr_in6 x);

bool lsbelong(struct sockaddr_in6 addr, lserv servs);

bool lsempty(lserv l);

void lsfprint(FILE *stream, lserv l);

#endif
