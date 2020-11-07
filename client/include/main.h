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

#include "macro.h"
#include "parser.h"

#define REQLEN 512

struct request {
    int id;
    char name[NAMELEN];
    struct tab_addr tab_dst;
    size_t index_addr;
};

struct tab_requests {
    struct request requests[MAX_REQUESTS];
    int nb_requests;
};

void ignore(struct sockaddr_in6 addr, struct ignored_servers *servers);
