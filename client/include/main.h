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

#define REQ_MAX 512
#define NAME_MAX 124

char *resolve(int soc_v4, int soc_v6, int *id, char *name, struct addr_with_flag *tab_addr, bool free_tab);