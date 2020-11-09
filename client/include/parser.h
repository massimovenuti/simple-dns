#ifndef __PARSER_H__
#define __PARSER_H__

#include "addr.h"
#include "macro.h"
#include "req.h"

#define INCREASE_COEF 3

#define TIMELEN 200
#define CODELEN 2
#define IDLEN 129
#define PORTLEN 10
#define SEPARATOR "|"

struct res {
    int id;
    struct timeval time;
    char req_name[100];
    int code;
    char name[100];
    struct tab_addrs addrs;
};

struct sockaddr_in6 convert(char ip[], int port);

struct tab_addrs parse_conf(const char *file_name);

struct res parse_res(char *res);

#endif
