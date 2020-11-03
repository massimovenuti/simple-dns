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

char *resolve(int soc, int *id, char *name, struct addr_with_flag *tab_addr, bool free_tab);