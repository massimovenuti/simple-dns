/**
 * @file main.h
 * @author Alexandre Vogel, Massimo Venuti
 * @brief Serveur de nom DNS - fichier en-tête
 * @date 2020-11-16
 * 
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "ack.h"
#include "macro.h"
#include "parser.h"

#define REQLEN 512
#define RESLEN 1024
#define TIMEOUT 5

/**
 * @brief Traite une requête
 * 
 * Récupère une requête puis crée et envoie la réponse.
 */
void processes_request(int soc, struct tab_names tab_of_addr, lack* ack_wait);

/**
 * @brief Teste si un acquittement est en timeout
 */
bool timeout(struct ack a);

/**
 * @brief 
 */
void tchk_ack(lack* l, int soc, struct tab_names tab_of_addr);


#endif
