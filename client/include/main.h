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

// code = 1 found
// code = -1 not found
// code = 0 timeout

bool is_ignored(laddr l, struct sockaddr_in6 addr);

bool timeout(struct req req);

void check_timeout(int soc, lreq *lr, laddr *suspicious, laddr *ignored,
                   laddr monitored, bool monitoring);

void handle_request(char *input, int soc, int *id, lreq *lr,
                    struct tab_addrs roots, laddr ignored, bool monitoring);

void handle_monitoring(laddr *monitored, bool *monitoring);

void handle_ignored(laddr ignored);

void handle_status(laddr ignored, laddr suspicious, laddr monitored,
                   bool monitoring);

void handle_unknown(char *input);

void handle_reqfile(const char *path, int soc, int *id, lreq *reqs,
                    struct tab_addrs *roots, laddr ignored, laddr suspicious,
                    laddr *monitored, bool *goon, bool *monitoring);

void handle_command(char *command, int soc, int *id, lreq *reqs,
                    struct tab_addrs *roots, laddr ignored, laddr suspicious,
                    laddr *monitored, bool *goon, bool *monitoring);

void read_input(FILE *stream, int soc, int *id, lreq *reqs,
                struct tab_addrs *roots, laddr ignored, laddr suspicious,
                laddr *monitored, bool *goon, bool *monitoring);

void read_network(int soc, int *id, lreq *reqs, laddr ignored, laddr *monitored,
                  bool monitoring);

void print_res(char *req, struct tab_addrs ta);

bool send_req(int soc, struct req *req, laddr ignored, bool monitoring);

struct res receive_res(int soc, laddr *monitored, bool monitoring);

#endif
