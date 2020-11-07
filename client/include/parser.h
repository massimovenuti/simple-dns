#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

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
    struct timeval time;
    char req_name[100];
    int code;
    char name[100];
    struct addr_with_flag *addrs;
};

struct tree {
    char name[NAMELEN];
    struct addr_with_flag tab_addr[MAX_ADDR]; // à allouer dynamiquement...
    int nb_addrs;
    int index;
    struct tree *sons;
    int nb_sons;
};

bool is_ignored(char *ip, char *port, struct ignored_servers servers);

void convert(char ip[], int port, struct sockaddr_in6 *dst);

struct addr_with_flag *parse_conf(const char *file_name);

struct res parse_res(char *res, size_t len, struct ignored_servers servers);

struct server addr_to_string(struct addr_with_flag addr);

struct tree *rech_inter(char *name, struct tree *t, int ind);

struct tree *rech(char *name, struct tree *t);
