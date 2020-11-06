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


struct request {
    int id;
    char name[NAMELEN];
};

struct tab_requests {
    struct request requests[MAX_REQUESTS];
    int nb_requests;
};

char *resolve(int soc, struct request *request, char *dst, struct tree *tree_addr, struct ignored_servers *ignored_serv, bool monitoring, bool free_tab);

void ignore(struct addr_with_flag addr, struct ignored_servers *servers);
