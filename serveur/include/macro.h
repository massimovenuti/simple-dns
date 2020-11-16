/**
 * @file macro.h
 * @author Alexandre Vogel, Massimo Venuti
 * @brief Définition de macros
 * @date 2020-11-16
 *
 */

#ifndef __MACRO_H__
#define __MACRO_H__

#define TCHK(exp) do { \
    if ((errno = (exp)) > 0) \
        {perror(#exp); exit(1);} \
    } while(0)

#define BCHK(exp) do { \
    if ((errno = (exp)) > 0) \
        {if (errno != PTHREAD_BARRIER_SERIAL_THREAD){perror(#exp); exit(1);}} \
    } while(0)

#define MCHK(exp) do { \
    if ((exp) == NULL) \
        {perror(#exp); exit(1);} \
    } while(0)

#define PCHK(exp) do { \
    if ((exp) == -1) \
        {perror(#exp); exit(1);} \
    } while(0)

#endif
