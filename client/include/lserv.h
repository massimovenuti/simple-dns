/**
 * @file lserv.h
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Gestion des serveurs - fichier en-tête
 * @date 2020-11-16
 * 
 */
#ifndef __LSERV_H__
#define __LSERV_H__

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "lreq.h"
#include "macro.h"
#include "mytime.h"

/**
 * @brief Objet représentant un serveur
 *
 * Caractérisé par une adresse, un nombre d'envois et de réceptions et un temps
 * moyen de réponse.
 *
 */
struct server {
    struct sockaddr_in6 addr;
    int nb_replies;
    int nb_shipments;
    struct timeval avg_reply_time;
};

/**
 * @brief Liste chaînée de serveurs
 */
typedef struct s_lserv {
    struct server server;
    struct s_lserv *next;
} * lserv;

/**
 * @brief Incrémente le nombre d'envois vers un serveur
 */
void add_shipment(struct server *s);

/**
 * @brief Incrémente le nombre de réponses d'un serveur
 */
void add_reply(struct server *s, struct timeval t);

/**
 * @brief Crée un nouveau serveur
 */
struct server new_server(struct sockaddr_in6 a);

/**
 * @brief Affiche une adresse
 *
 * Affiche une adresse au format ip:port.
 */
void afprint(FILE *stream, struct sockaddr_in6 addr);

/**
 * @brief Affiche un serveur
 *
 * Affiche l'adresse d'un serveur au format "ip:port" et les informations
 * associées au serveur.
 */
void sfprint(FILE *stream, struct server serv); /* /!\ à compléter */

/**
 * @brief Crée une nouvelle liste de serveurs
 */
lserv lsnew();

/**
 * @brief Libère une liste de serveurs
 */
void lsfree(lserv l);

/**
 * @brief Ajoute un serveur à une liste de serveurs
 */
lserv lsadd(lserv l, struct server x);

/**
 * @brief Supprime un serveur d'une liste de serveurs
 */
lserv lsrm(lserv l, struct sockaddr_in6 x);

/**
 * @brief Supprime la tête d'une liste de serveurs
 */
lserv lsrmh(lserv l);

/**
 * @brief Renvoie la longueur d'une liste de serveurs
 */
int lslen(lserv l);

/**
 * @brief Cherche un serveur dans une liste de serveurs
 *
 * Renvoie le maillon correspondant au serveur recherché dans la liste si il
 * existe et le maillon vide (NULL) sinon.
 */
lserv lssearch(lserv l, struct sockaddr_in6 x);

/**
 * @brief Teste si une adresse est présente dans une lise de serveurs
 */
bool lsbelong(struct sockaddr_in6 addr, lserv servs);

/**
 * @brief Teste si une liste de serveurs est vide
 */
bool lsempty(lserv l);

/**
 * @brief Vide une liste de serveurs
 */
lserv reset(lserv l);

/**
 * @brief Affiche les serveurs d'une liste de serveurs
 *
 * Affiche les adresses des serveurs d'une liste de serveurs au format "ip:port"
 * ainsi que les informations associées à chaque serveur.
 */
void lsfprint(FILE *stream, lserv l);

#endif
