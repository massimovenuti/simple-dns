/**
 * @file macro.h
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Macros de test
 * @date 2020-11-16
 * 
 */

#define TCHK(exp) do { \
    if ((errno = (exp)) > 0) \
        {perror(#exp); exit(1);} \
    } while(0)

#define MCHK(exp) do { \
    if ((exp) == NULL) \
        {perror(#exp); exit(1);} \
    } while(0)

#define PCHK(exp) do { \
    if ((exp) == -1) \
        {perror(#exp); exit(1);} \
    } while(0)
