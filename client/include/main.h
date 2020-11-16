/**
 * @file main.h
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Client - fichier en-tête
 * @date 2020-11-16
 *
 */
#ifndef __MAIN_H__
#define __MAIN_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "lreq.h"
#include "lserv.h"
#include "macro.h"
#include "mytime.h"
#include "parser.h"

#define TIMEOUT 2
#define RESET_TIME 10

#define PATHLEN 256

/**
 * @brief Surveille la réponse d'un serveur
 *
 * Met à jour les informations d'un serveur après une réception par ce dernier
 * et affiche la réponse et le temps de réponse du serveur.
 */
void monitor_reply(lserv *monitored, struct res res, struct sockaddr_in6 addr);

/**
 * @brief Surveille l'envoi d'une requête
 *
 * Met à jour les informations d'un serveur après un envoi vers ce dernier et
 * affiche la requête ainsi que l'adresse du serveur.
 */
void monitor_shipment(lserv *monitored, char *req, struct sockaddr_in6 addr);

/**
 * @brief Traite une requête qui a timeout
 */
bool handle_timeout(int soc, struct req *req, struct sockaddr_in6 addr,
                    lreq *reqs, lserv *ignored, lserv *suspicious,
                    lserv *monitored, bool monitoring);

/**
 * @brief Vérifie si des requêtes ont timeout
 */
void check_timeout(int soc, struct timeval *reset_t, lreq *reqs,
                   lserv *suspicious, lserv *ignored, lserv *monitored,
                   bool monitoring);

/**
 * @brief Reçoit la réponse d'un serveur
 *
 * Envoie un acquittement au serveur après bonne réception.
 */
struct res receive_reply(int soc, lserv *monitored, bool monitoring);

/**
 * @brief Traite la réponse d'un serveur
 */
void handle_reply(int soc, int *id, lreq *reqs, struct req *req, struct res res,
                  lserv ignored, lserv *monitored, bool monitoring);

/**
 * @brief Envoie un acquittement à un serveur
 */
void send_ack(int soc, int id, struct sockaddr_in6 addr);

/**
 * @brief Convertit une structure req en chaîne de caractère
 */
int reqtostr(struct req s_req, char *str_req);

/**
 * @brief Envoie une requête à un serveur
 */
bool send_request(int soc, struct req *s_req, lserv ignored, lserv *monitored,
                  bool monitoring);

/**
 * @brief Traite une nouvelle requête
 */
void handle_request(char *input, int soc, int *id, lreq *reqs,
                    struct tab_addrs roots, lserv ignored, lserv *monitored,
                    bool monitoring);

/**
 * @brief Affiche la liste des commandes disponibles
 */
void print_help();

/**
 * @brief Charge un fichier de requêtes
 *
 * Envoie les requêtes et commandes présentes dans un fichier de requêtes au
 * programme.
 */
void load_reqfile(const char *path, int soc, int *id, lreq *reqs,
                  struct tab_addrs *roots, lserv *ignored, lserv *suspicious,
                  lserv *monitored, bool *goon, bool *monitoring);

/**
 * @brief Traite une commande utilisateur transmise sur l'entrée standard
 */
void handle_command(char *command, int soc, int *id, lreq *reqs,
                    struct tab_addrs *roots, lserv *ignored, lserv *suspicious,
                    lserv *monitored, bool *goon, bool *monitoring);

/**
 * @brief Lit l'entrée standard
 */
void read_input(FILE *stream, int soc, int *id, lreq *reqs,
                struct tab_addrs *roots, lserv *ignored, lserv *suspicious,
                lserv *monitored, bool *goon, bool *monitoring);

/**
 * @brief Lit le réseau
 */
void read_network(int soc, int *id, lreq *reqs, lserv ignored, lserv *monitored,
                  bool monitoring);

#endif
