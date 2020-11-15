#ifndef __PARSER_H__
#define __PARSER_H__

#include <string.h>

#include "lreq.h"
#include "macro.h"
#include "mytime.h"

#define INCREASE_COEF 3

#define LTIME 30
#define LRES 724
#define LCODE 2
#define LID 32
#define SEPARATOR "|"

struct res {
    int id;
    struct timeval time;
    char req_name[LREQ];
    int code;
    char name[LNAME];
    struct tab_addrs addrs;
};

struct sockaddr_in6 convert(char ip[], int port);

struct tab_addrs parse_conf(const char *file_name);

void update_restime(struct res *res);

void parse_addrs(struct res *res, char *addrs) ;

struct res parse_res(char *res);

#endif
