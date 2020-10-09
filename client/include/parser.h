#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define TABSIZE 100

struct addr_with_flag {
    struct sockaddr addr;
    bool ignore;
    bool end;
};

void convert(char ip[], int port, struct sockaddr *dst);
struct addr_with_flag *parse(const char *file_name);