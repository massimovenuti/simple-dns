/**
 * @file main.h
 * @author Alexandre Vogel, Massimo Venuti
 * @brief Client - fichier en-tête
 * @date 2020-11-16
 * 
 */

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

#include "macro.h"
#include "parser.h"

#define REQLEN 512

char *resolve(int soc, int *id, char *name, char *dst, struct addr_with_flag *tab_addr, struct ignored_servers *ignored_serv, bool monitoring, bool free_tab);
void ignore(struct addr_with_flag addr, struct ignored_servers *servers);
