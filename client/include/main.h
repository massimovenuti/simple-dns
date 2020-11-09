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

#define REQLEN 512

bool is_ignored(laddr l, struct sockaddr_in6 addr);

void send_req(int soc, struct req *req, laddr ignored, bool monitoring);

struct res receive_res(int soc, laddr ignored, bool monitoring);

void read_input(FILE *stream, int soc, int *id, lreq *reqs, struct tab_addrs root_addr, laddr ignored, bool *goon, bool *monitoring);

void read_network(int soc, int *id, lreq *reqs, laddr ignored, bool monitoring);

#endif
