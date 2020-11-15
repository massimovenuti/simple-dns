#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

struct tab_servers {
    struct server *servs;
    int max_len;
    int len;
};

struct name {
    char name[NAMELEN];
    struct tab_servers tab_servs;
};

struct tab_names {
    struct name *names;
    int len;
    int max_len;
};

struct tab_servers new_tab_servers();

struct tab_names new_tab_names();

struct server new_server();

struct name new_name();

void free_tab_servers(struct tab_servers *s);

void free_tab_names(struct tab_names *n);

void add_name(struct tab_names *tn, struct name n);

void add_server(struct tab_servers *ts, struct server s);

int search_name(struct tab_names n, char *name);

void *inctab(void *dest, int len, int *max_len, size_t size_elem, int coef);

char *incstr(char *dest, size_t len, size_t *max_len, int coef);

bool compare(char *s1, char *s2);

struct tab_names parse_conf(const char *file_name);

bool parse_req(char *dest, char *src);

bool make_res(char *dest, char *src, struct tab_names n, size_t *len_dest, size_t len_src, size_t *max_len_dest);

#endif
