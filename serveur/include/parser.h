
/**
 * @file parser.h
 * @author Alexandre Vogel, Massimo Venuti
 * @brief Parser côté serveur - fichier en-tête
 * @date 2020-11-16
 *
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

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
 * @brief Objet représentant un serveur
 *
 * Caractérisé par une ip et un port.
 */
struct server
{
    char ip[IPLEN];
    char port[PORTLEN];
};

/**
 * @brief Objet représentant un tableau de serveurs
 */
struct tab_servers
{
    struct server *servs;
    int max_len;
    int len;
};

/**
 * @brief Objet représentant un nom
 *
 * Caractérisé par un nom et un tableau de serveurs associé.
 */
struct name
{
    char name[NAMELEN];
    struct tab_servers tab_servs;
};

/**
 * @brief Objet représentant un tableau de noms
 */
struct tab_names
{
    struct name *names;
    int max_len;
    int len;
};

/**
 * @brief Objet représentant une requête client
 */
struct req
{
    int id;
    struct timeval time;
    char name[NAMELEN];
};

/**
 * @brief Crée et initialise un tableau de serveurs vide
 */
struct tab_servers new_tab_servers ();

/**
 * @brief Crée et initialise un tableau de noms vide
 */
struct tab_names new_tab_names ();

/**
 * @brief Crée un nouveau serveur représenté par une ip et un port
 */
struct server new_server (char *ip, char *port);

/**
 * @brief Crée un nouveau nom avec un tableau de serveurs associé
 */
struct name new_name (char *name, struct tab_servers servs);

/**
 * @brief Libère un tableau de serveurs
 */
void free_tab_servers (struct tab_servers *s);

/**
 * @brief Libère un tableau de noms
 */
void free_tab_names (struct tab_names *n);

/**
 * @brief Ajoute un nom à un tableau de noms
 */
void add_name (struct tab_names *tn, struct name n);

/**
 * @brief Ajoute un serveur à un tableau de serveurs
 */
void add_server (struct tab_servers *ts, struct server s);

/**
 * @brief Renvoie l'indice d'un nom dans un tableau de noms
 *
 * Si le nom n'appartient pas au tableau de noms, la fonction renvoie -1.
 */
int search_name (struct tab_names n, char *name);

/**
 * @brief Augmente la taille mémoire d'un tableau
 *
 * Augmente la taille mémoire d'un tableau d'éléments quelconques avec une
 * taille souhaitée. Si la taille souhaitée est plus petite que la taille
 * actuelle du tableau, rien n'est modifié.
 *
 * @param dest Tableau à tester
 * @param len Nouvelle taille souhaitée
 * @param max_len Taille maximale de dest - modifiée par effet de bord
 * @param size_elem Taille d'un élément du tableau
 * @return true Si la taille mémoire de dest a été augmentée
 * @return false Sinon
 */
void *inctab (void *dest, int len, int *max_len, size_t size_elem, int coef);

/**
 * @brief Augmente la taille mémoire d'une chaîne de caractères
 *
 * Augmente la taille mémoire d'une chaîne de caractères avec une taille
 * souhaitée. Si la taille souhaitée est plus petite que la taille actuelle de
 * la chaîne, rien n'est modifié.
 *
 * @param dest Chaîne de caractères à tester
 * @param len Nouvelle taille souhaitée
 * @param max_len Taille maximale de dest - modifiée par effet de bord
 * @return true Si la taille mémoire de dest a été augmentée
 * @return false Sinon
 */
char *incstr (char *dest, size_t len, size_t *max_len, int coef);

/**
 * @brief Compare deux chaînes de caractères
 *
 * Test si deux chaînes de caractères sont identiques sans prendre en compte un
 * éventuel caractère "." devant la seconde chaîne.
 *
 * Exemple: compare("fr", ".fr") renvoie vrai
 */
bool compare (char *s1, char *s2);

/**
 * @brief Parse un fichier de configuration
 *
 * Renvoie la liste des noms et serveurs associés d'un fichier de configuration
 * sous la forme d'un tableau de noms.
 */
struct tab_names parse_conf (const char *file_name);

/**
 * @brief Parse une requête
 *
 * Parse une requête de la forme <id>|<time>|<nom>
 */
struct req parse_req (char *src);

/**
 * @brief Teste si une chaîne de caractères est un acquittement
 */
int is_ack (char str[]);

/**
 * @brief Construit la réponse à une requête
 *
 * Construit une réponse de la forme <req>|<code>|[<result>]
 */
char *make_res (char *dest, struct req req, struct tab_names n,
		size_t *len_dest, size_t len_src, size_t *max_len_dest);

#endif
