
/**
 * @file lreq.h
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Gestion des requêtes - fichier en-tête
 * @date 2020-11-16
 * 
 */
#ifndef __LREQ_H__
#define __LREQ_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "macro.h"

#define TABSIZE 100

#define LREQ 512
#define LNAME 256
#define LIP 140
#define LPORT 10

/**
 * @brief Objet représentant un tableau d'adresses
 */
struct tab_addrs
{
    struct sockaddr_in6 addrs[TABSIZE];
    int len;
};

/**
 * @brief Objet représentant une requête pour un serveur
 *
 * Caractérisée par un id, un nom à trouver, un tableau de serveurs
 * correspondant et le moment ou la requête est envoyée.
 */
struct req
{
    int id;
    char name[LNAME];
    struct tab_addrs dest_addrs;
    struct timeval t;
    int index;
};

/**
 * @brief Liste chaînée de requêtes
 */
typedef struct s_lreq
{
    struct req req;
    struct s_lreq *next;
} *lreq;

/**
 * @brief Crée une nouvelle requête
 *
 * Crée une nouvelle requête en fixant l'index du serveur courant
 * automatiquement pour vérifier la règle du tourniquet.
 */
struct req *new_req (lreq * l, int id, char *name, struct tab_addrs addrs);

/**
 * @brief Met à jour une requête
 *
 * Met à jour une requête avec un nouvel id et un nouveau tableau d'adresses.
 * L'index du serveur courant est modifié automatiquement pour vérifier la règle
 * du tourniquet.
 */
void update_req (lreq * l, struct req *req, int id, struct tab_addrs addrs);

/**
 * @brief Récupère l'index du serveur courant d'une requête
 */
int get_index (lreq l, struct req req);

/**
 * @brief Teste si deux adresses sont identiques
 */
bool addrcmp (struct sockaddr_in6 a1, struct sockaddr_in6 a2);

/**
 * @brief Teste si une adresse appartient à un tableau d'adresses
 */
bool belong (struct sockaddr_in6 addr, struct tab_addrs addrs);

/**
 * @brief Créer une nouvelle liste de requêtes
 */
lreq lrnew ();

/**
 * @brief Libère une liste de requêtes
 */
void lrfree (lreq l);

/**
 * @brief Ajoute une requête à une liste de requêtes
 */
lreq lradd (lreq * l, struct req x);

/**
 * @brief Supprime une requête d'une liste de requêtes
 */
lreq lrrm (lreq l, int id);

/**
 * @brief Cherche une requête dans une liste de requêtes
 *
 * Renvoie le maillon correspondant à la requête recherchée dans la liste si il
 * existe et le maillon vide (NULL) sinon.
 */
lreq lrsearch (lreq l, int id);

/**
 * @brief Teste si une liste de requêtes est vide
 */
bool lrempty (lreq l);

#endif
