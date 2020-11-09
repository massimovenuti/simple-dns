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


struct tab_addrs {
    struct sockaddr_in6 *addrs;
    int len;
};

struct res {
    int id;
    struct timeval time;
    char req_name[100];
    int code;
    char name[100];
    struct tab_addrs addrs;
};

void convert(char ip[], int port, struct sockaddr_in6 *dst);

struct tab_addrs parse_conf(const char *file_name);

struct res parse_res(char *res);

void fprint_addr(FILE *stream, struct sockaddr_in6 addr);