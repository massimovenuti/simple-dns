#ifndef __REQ_H__
#define __REQ_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "addr.h"
#include "macro.h"

#define NAMELEN 256

struct req {
    int id;
    char name[NAMELEN];
    struct tab_addrs dest_addrs;
    struct timeval t;
    int index;
};

typedef struct s_lreq {
    struct req req;
    struct s_lreq *next;
} * lreq;

struct req* new_req(lreq *l, int id, char *name, struct tab_addrs addrs);

void update_req(lreq *l, struct req *req, int id, struct tab_addrs addrs);

int get_index(lreq l, struct req req);

lreq lreq_new();

void lreq_destroy(lreq l);

lreq lreq_add(lreq *l, struct req x);

lreq lreq_rm(lreq l, int id);

struct req lreq_elem(lreq l, int i);

int lreq_len(lreq l);

lreq lreq_search(lreq l, int id);

bool lreq_empty(lreq l);

#endif
