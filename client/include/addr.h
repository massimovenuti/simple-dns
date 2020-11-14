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
#include "time.h"

#define TABSIZE 100
#define IPLEN 140
#define PORTLEN 10

struct tab_addrs {
    struct sockaddr_in6 addrs[TABSIZE];
    int len;
};

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

struct server new_serv(struct sockaddr_in6 a);

lserv lserv_new();

void lserv_destroy(lserv l);

lserv lserv_add(lserv l, struct server x);

lserv lserv_rm(lserv l, struct sockaddr_in6 x);

struct server lserv_elem(lserv l, int i);

int lserv_len(lserv l);

lserv lserv_search(lserv l, struct sockaddr_in6 x);

bool lserv_belong(struct sockaddr_in6 addr, lserv servs);

bool lserv_empty(lserv l);

void use(struct server *a, struct timeval t);

bool addr_cmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2);

bool addr_in(struct sockaddr_in6 addr, struct tab_addrs addrs);

void lserv_fprint(FILE *stream, lserv l);

void fprint_serv(FILE *stream, struct server serv); /* /!\ à compléter */

void fprint_addr(FILE *stream, struct sockaddr_in6 addr);

#endif
