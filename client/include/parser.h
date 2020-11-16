
/**
 * @file parser.h
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Parser côté client - fichier en-tête
 * @date 2020-11-16
 *
 */
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

/**
 * @brief Objet représentant le résultat d'un serveur pour une requête
 *
 * Caractérisé par un id de requête, le nom recherché, le temps de réponse du
 * serveur, le code de réponse du serveur, le nom que le serveur a trouvé et
 * les adresses associées.
 */
struct res
{
    int id;
    char req_name[LNAME];
    struct timeval time;
    int code;
    char name[LNAME];
    struct tab_addrs addrs;
};

/**
 * @brief Crée une structure sockaddr_in6 à partir de chaînes de caractères
 *
 * Crée une structure sockaddr_in6 à partir d'une chaîne de caractères
 * représentant l'adresse ip et d'une autre représenant le port.
 */
struct sockaddr_in6 convert (char ip[], int port);

/**
 * @brief Parse un fichier de configuration
 *
 * Renvoie la liste des adresses d'un fichier de configuration sous la forme
 * d'un tableau d'adresses.
 */
struct tab_addrs parse_conf (const char *file_name);

/**
 * @brief Met à jour le temps d'une structure res
 *
 * Met à jour le temps d'une strucure res en le soustrayant au temps actuel.
 */
void update_restime (struct res *res);

/**
 * @brief Parse la liste d'adresses envoyé par un serveur
 */
void parse_addrs (struct res *res, char *addrs);

/**
 * @brief Parse le résultat d'un serveur
 */
struct res parse_res (char *res);

#endif
