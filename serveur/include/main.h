/**
 * @file main.h
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Serveur de nom DNS - fichier en-tête
 * @date 2020-11-16
 * 
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "macro.h"
#include "parser.h"

#define REQLEN 512
#define RESLEN 1024

/**
 * @brief Objet contenant les arguments d'un thread
 * 
 */
struct thread_arg {
    int soc;
    struct name *tab_of_addr;
};
