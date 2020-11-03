#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "macro.h"

#define TABSIZE 100
#define INCREASE_COEF 3

#define NAMELEN 256
#define IPLEN 140
#define TIMELEN 200
#define CODELEN 2
#define IDLEN 129
#define PORTLEN 10

#define SEPARATOR "|"

struct addr_with_flag {
    struct sockaddr_in6 addr;
    bool ignore;
    bool end;
};

struct res
{
    int id;
    time_t time;
    char req_name[100];
    int code;
    char name[100];
    struct addr_with_flag *addrs; //passer en dynamique
};


void convert(char ip[], int port, struct sockaddr_in6 *dst);
struct addr_with_flag *parse_conf(const char *file_name);

struct res parse_res(char *res, size_t len);