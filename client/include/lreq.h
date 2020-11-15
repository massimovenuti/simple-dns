#ifndef __REQ_H__
#define __REQ_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "macro.h"

#define TABSIZE 100

#define LNAME 256
#define LIP 140
#define LPORT 10
#define LREQ 512

struct tab_addrs {
    struct sockaddr_in6 addrs[TABSIZE];
    int len;
};

struct req {
    int id;
    char name[LNAME];
    struct tab_addrs dest_addrs;
    struct timeval t;
    int index;
};

typedef struct s_lreq {
    struct req req;
    struct s_lreq *next;
} * lreq;

struct req *new_req(lreq *l, int id, char *name, struct tab_addrs addrs);

void update_req(lreq *l, struct req *req, int id, struct tab_addrs addrs);

int get_index(lreq l, struct req req);

bool addrcmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2);

bool belong(struct sockaddr_in6 addr, struct tab_addrs addrs);

lreq lrnew();

void lrfree(lreq l);

lreq lradd(lreq *l, struct req x);

lreq lrrm(lreq l, int id);

int lrlen(lreq l);

lreq lrsearch(lreq l, int id);

bool lrempty(lreq l);

#endif