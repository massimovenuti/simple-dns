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

#include "time.h"
#include "macro.h"

#define TABSIZE 100
#define IPLEN 140
#define PORTLEN 10

struct tab_addrs {
    struct sockaddr_in6 addrs[TABSIZE];
    int len;
};

struct monitored_addr {
    struct sockaddr_in6 addr;
    int counter;
    struct timeval avg_time;
};

typedef struct s_laddr {
    struct monitored_addr m_addr;
    struct s_laddr *next;
} * laddr;

bool addr_cmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2);

bool addr_in(struct sockaddr_in6 addr, struct tab_addrs addrs);

void fprint_addr(FILE *stream, struct sockaddr_in6 addr);

struct monitored_addr new_maddr(struct sockaddr_in6 a);

void use(struct monitored_addr *a, struct timeval t);

laddr laddr_new();

void laddr_destroy(laddr l);

laddr laddr_add(laddr l, struct monitored_addr x);

laddr laddr_rm(laddr l, struct sockaddr_in6 x);

struct monitored_addr laddr_elem(laddr l, int i);

int laddr_len(laddr l);

laddr laddr_search(laddr l, struct sockaddr_in6 x);

bool laddr_empty(laddr l);

void laddr_fprint(FILE *stream, laddr l);

#endif
