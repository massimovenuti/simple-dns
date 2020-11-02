#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "macro.h"

#define TABSIZE 100
#define INCREASE_COEF 3

#define NAMELEN 256
#define IPLEN 140
#define PORTLEN 10

#define SUCCESS "1"
#define FAIL "-1"
#define SEPARATOR "|"
#define SUBSEPARATOR ","

struct server {
    char ip[IPLEN];
    char port[PORTLEN];
};

struct name {
    char name[NAMELEN];
    struct server *servers; //passer en dynamique
    int nb_servers;
    int max_servers;
};

int compare(char *s1, char *s2);

struct name *parse_conf(const char *file_name);

char *parse_req(char *str, size_t len);

char *make_res(struct name *names, char *req, size_t *len);
