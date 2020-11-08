/**
 * @file parser.h
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Parser côté serveur - fichier en-tête
 * @date 2020-11-16
 * 
 */

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

/**
 * @brief Objet représentant un serveur de nom
 * 
 */
struct server {
    char ip[IPLEN];
    char port[PORTLEN];
};

/**
 * @brief Objet représentant un nom de domaine
 * 
 */
struct name {
    char name[NAMELEN];
    struct server *servers;
    int nb_servers;
    int max_servers;
};

void init_names(struct name *names, int start, int end);

void free_names(struct name *names);

bool compare(char *s1, char *s2);

struct name *parse_conf(const char *file_name);

bool parse_req(char *dest, char *src);

bool increase_memsize(char *dest, size_t *size_dest, size_t size_src);

bool make_res(char *dest, char *src, struct name *names, size_t *len_dest, size_t len_src, size_t *max_len_dest);
