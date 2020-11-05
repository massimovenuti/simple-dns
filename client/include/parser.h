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

#define MAX_IGNORED 10

#define NAMELEN 256
#define IPLEN 140
#define TIMELEN 200
#define CODELEN 2
#define IDLEN 129
#define PORTLEN 10

#define SEPARATOR "|"

struct addr_with_flag {
    struct sockaddr_in6 addr;
    bool ignore; // à enlever
    bool end;
};

struct server {
    char ip[IPLEN];
    char port[PORTLEN];
};

struct ignored_servers {
    struct server servers[10];
    int nb_servers;
};

struct res
{
    int id;
    time_t time;
    char req_name[100];
    int code;
    char name[100];
    struct addr_with_flag *addrs;
};

bool is_ignored(char *ip, char *port, struct ignored_servers servers);
void convert(char ip[], int port, struct sockaddr_in6 *dst);
struct addr_with_flag *parse_conf(const char *file_name);
struct res parse_res(char *res, size_t len, struct ignored_servers servers);
struct server addr_to_string(struct addr_with_flag addr);
