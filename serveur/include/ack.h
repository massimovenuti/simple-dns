/**
 * @file ack.h
 * @author Alexandre Vogel, Massimo Venuti
 * @brief Acquittements clients attendus - fichier en-tête 
 * @date 2020-11-16
 * 
 */

#ifndef __ACK_H__
#define __ACK_H__

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "macro.h"
#include "parser.h"

/**
 * @brief Objet représentant un acquittement client attendu
 */
struct ack {
    struct req req;
    struct sockaddr_in6 addr;
    struct timeval time;
    int retry;
};

/**
 * @brief Liste chainée d'acquittements
 */
typedef struct s_lack {
    struct ack ack;
    struct s_lack *next;
} * lack;

/**
 * @brief Créer une nouvelle liste d'acquittements
 */
lack lack_new();

/**
 * @brief Libère une liste d'acquittements
 */
void lack_destroy(lack l);

/**
 * @brief Ajoute un acquittement à une liste d'acquittements
 */
lack lack_add(lack l, struct req req, struct sockaddr_in6 addr);

/**
 * @brief Supprime un acquittement d'une liste d'acquittements
 */
lack lack_rm(lack l, int id, struct sockaddr_in6 addr);

/**
 * @brief Teste si une liste d'acquittements est vide
 */
bool lack_empty(lack l);

/**
 * @brief Teste si deux addresses sont identiques
 */
bool addr_cmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2);

#endif
