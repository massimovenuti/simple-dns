/**
 * @file mytime.h
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Gestion du temps - fichier en-tête
 * @date 2020-11-16
 * 
 */
#ifndef __TIME_H__
#define __TIME_H__

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "macro.h"

/**
 * @brief Crée une structure timeval
 *
 * Crée une structure timeval et l'initialise avec les valeurs indiquées.
 */
struct timeval new_timeval(time_t sec, time_t usec);

/**
 * @brief Crée un compte à rebours
 *
 * Crée une structure timeval et l'initialise avec la date courante à laquelle
 * est ajoutée sec secondes et usec microsecondes.
 */
struct timeval new_countdown(int sec, int usec);

/**
 * @brief Teste si le temps d'une structure timeval est dépassé
 * 
 * Compare le temps indiqué par une structure timeval avec le temps courant.
 */
bool timeout(struct timeval t, int time);

/**
 * @brief Renvoie un temps en seconde à partir d'une structure timeval
 */
float get_timevalue(struct timeval t);

/**
 * @brief Réalise une opération arithmétique entre deux structures timeval
 * 
 * L'opérateur peut être '+' ou '-'.
 */
struct timeval op_timeval(struct timeval t1, struct timeval t2, char op);

/**
 * @brief Réalise une opération arithmétique sur une structure timeval
 * 
 * L'opérateur peut être '*' ou '-'.
 */
struct timeval op_ntimeval(struct timeval t, int n, char op);

#endif
