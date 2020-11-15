#ifndef __MACRO_H__
#define __MACRO_H__

#define TCHK(exp)                  \
    do {                           \
        if ((errno = (exp)) > 0) { \
            perror(#exp);          \
            exit(1);               \
        }                          \
    } while (0)

#define MCHK(exp)            \
    do {                     \
        if ((exp) == NULL) { \
            perror(#exp);    \
            exit(1);         \
        }                    \
    } while (0)

#define PCHK(exp)          \
    do {                   \
        if ((exp) == -1) { \
            perror(#exp);  \
            exit(1);       \
        }                  \
    } while (0)

#define str(s) #s

#define STR(s) str(s)

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

#define NEWLINE "\n"
#define TAB "\t"

#endif
