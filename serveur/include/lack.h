/**
 * @file lack.h
 * @author Alexandre Vogel, Massimo Venuti
 * @brief Gestion des acquittements - fichier en-tête 
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
 * @brief Crée une nouvelle liste d'acquittements
 */
lack lanew();

/**
 * @brief Libère une liste d'acquittements
 */
void ladestroy(lack l);

/**
 * @brief Ajoute un acquittement à une liste d'acquittements
 */
lack laadd(lack l, struct req req, struct sockaddr_in6 addr);

/**
 * @brief Supprime un acquittement d'une liste d'acquittements
 */
lack larm(lack l, int id, struct sockaddr_in6 addr);

/**
 * @brief Teste si une liste d'acquittements est vide
 */
bool laempty(lack l);

/**
 * @brief Teste si deux addresses sont identiques
 */
bool addrcmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2);

#endif
