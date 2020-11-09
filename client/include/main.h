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
    struct tab_addrs dest_addrs;
    int index;
};

struct tab_requests {
    struct request requests[MAX_REQUESTS];
    int len;
};

void init_tab_addrs(struct tab_addrs *a);

void init_tab_requests(struct tab_requests *r);

bool rm_request(struct tab_requests *r, int id);

bool addr_cmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2);

void ignore(struct sockaddr_in6 addr, struct tab_addrs *ignored);

bool is_ignored(struct sockaddr_in6 addr, struct tab_addrs ignored);

void send_req(int soc, struct request *request, struct tab_addrs ignored, struct tab_addrs *used, bool monitoring);

struct res receive_res(int soc, struct tab_addrs ignored, struct tab_addrs *used, bool monitoring);

void read_input(FILE *stream, int soc, int *id, struct tab_requests *tab_request, struct tab_addrs root_addr, struct tab_addrs ignored, struct tab_addrs *used, bool *goon, bool *monitoring);

void read_network(int soc, int *id, struct tab_requests *tab_request, struct tab_addrs root_addr, struct tab_addrs ignored, struct tab_addrs *used, bool *monitoring);
