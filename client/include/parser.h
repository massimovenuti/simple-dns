#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define TABSIZE 100

struct type_addr {
    sa_family_t type;
    struct sockaddr addr;
    bool ignore;
    bool end;
};

struct type_addr *parse(const char *file_name);