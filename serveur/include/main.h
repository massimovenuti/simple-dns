#ifndef __MAIN_H__
#define __MAIN_H__

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "ack.h"
#include "macro.h"
#include "parser.h"

#define REQLEN 512
#define RESLEN 1024

#endif
