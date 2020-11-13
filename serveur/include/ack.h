#ifndef __ACK_H__
#define __ACK_H__

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "macro.h"

struct ack {
    int id;
    struct sockaddr_in6 addr;

};

typedef struct s_lack {
    struct ack ack;
    struct s_lack *next;
} * lack;


lack lack_new();

void lack_destroy(lack l);

lack lack_add(lack l, int id, struct sockaddr_in6 addr);

lack lack_rm(lack l, int id, struct sockaddr_in6 addr);

bool lack_empty(lack l);

bool addr_cmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2);

#endif
