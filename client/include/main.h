#ifndef __MAIN_H__
#define __MAIN_H_

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

#include "addr.h"
#include "macro.h"
#include "parser.h"
#include "req.h"
#include "time.h"

#define REQLEN 512
#define TIMEOUT 5

struct running {
    bool goon;
    bool monitoring;
    bool interactive;
};

bool is_ignored(laddr l, struct sockaddr_in6 addr);

void send_req(int soc, struct req *req, laddr ignored, struct running run);

struct res receive_res(int soc, laddr *monitored, struct running run);

void read_input(FILE *stream, int soc, int *id, lreq *reqs, struct tab_addrs root_addr, laddr ignored, struct running *run);

void read_network(int soc, int *id, lreq *reqs, laddr ignored, laddr *monitored, struct running run);

#endif
