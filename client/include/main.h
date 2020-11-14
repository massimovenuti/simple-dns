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

/* Monitoring */

void monitor_timeout(char *req, struct sockaddr_in6 addr, bool first_timeout);

void monitor_reply(lserv *monitored, struct res res, struct sockaddr_in6 addr);

void monitor_shipment(lserv *monitored, char *req, struct sockaddr_in6 addr);

/* Timeout */

bool timeout(struct req req);

bool handle_timeout(int soc, struct req *req, struct sockaddr_in6 addr, lreq *reqs, lserv *ignored,
                    lserv *suspicious, lserv *monitored, bool monitoring);

void check_timeout(int soc, lreq *reqs, lserv *suspicious, lserv *ignored, lserv *monitored,
                   bool monitoring);

/* Reply */

struct res receive_reply(int soc, lserv *monitored, bool monitoring);

void handle_reply(int soc, int *id, lreq *reqs, struct req *req, struct res res, lserv ignored,
                  lserv *monitored, bool monitoring);

/* Shipment */

void send_ack(int soc, int id, struct sockaddr_in6 addr);

int reqtostr(struct req s_req, char *str_req);

bool send_request(int soc, struct req *s_req, lserv ignored, lserv *monitored, bool monitoring);

void handle_request(char *input, int soc, int *id, lreq *reqs, struct tab_addrs roots,
                    lserv ignored, lserv *monitored, bool monitoring);

/* Input */

void print_help();

void load_reqfile(const char *path, int soc, int *id, lreq *reqs, struct tab_addrs *roots,
                  lserv ignored, lserv suspicious, lserv *monitored, bool *goon, bool *monitoring);

void handle_command(char *command, int soc, int *id, lreq *reqs, struct tab_addrs *roots,
                    lserv ignored, lserv suspicious, lserv *monitored, bool *goon,
                    bool *monitoring);

void read_input(FILE *stream, int soc, int *id, lreq *reqs, struct tab_addrs *roots, lserv ignored,
                lserv suspicious, lserv *monitored, bool *goon, bool *monitoring);

void read_network(int soc, int *id, lreq *reqs, lserv ignored, lserv *monitored, bool monitoring);

#endif
