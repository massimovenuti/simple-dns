#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "macro.h"

#define TABSIZE 100
#define MAX_ADDR 50
#define MAX_SONS 4
#define INCREASE_COEF 3

#define MAX_IGNORED 10
#define MAX_REQUESTS 5

#define NAMELEN 256
#define IPLEN 140
#define TIMELEN 200
#define CODELEN 2
#define IDLEN 129
#define PORTLEN 10

#define SEPARATOR "|"

struct tab_addr {
    struct sockaddr_in6 *addr;
    size_t len;
};

struct server {
    char ip[IPLEN];
    char port[PORTLEN];
};

struct ignored_servers {
    struct server servers[10];
    int nb_servers;
};

struct res {
    int id;
    struct timeval time;
    char req_name[100];
    int code;
    char name[100];
    struct tab_addr addrs;
};

bool is_ignored(char *ip, char *port, struct ignored_servers servers);

void convert(char ip[], int port, struct sockaddr_in6 *dst);

struct tab_addr parse_conf(const char *file_name);

struct res parse_res(char *res, size_t len, struct ignored_servers servers);

struct server addr_to_string(struct sockaddr_in6 addr);
