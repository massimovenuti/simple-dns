#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "macro.h"

#define TABSIZE 100

struct addr_with_flag {
    struct sockaddr addr;
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
    struct addr_with_flag addrs[100];
};


void convert(char ip[], int port, struct sockaddr *dst);
struct addr_with_flag *parse_conf(const char *file_name);

struct res parse_res(char *res, size_t len);