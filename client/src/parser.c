#include "parser.h"

void convert(char ip[], int port, struct sockaddr *dst) {
    int check;
    struct sockaddr_in *ipV4addr = (struct sockaddr_in *)(dst);
    PCHK(check = inet_pton(AF_INET, ip, &ipV4addr->sin_addr));

    if (check == 0) {
        struct sockaddr_in6 *ipV6addr = (struct sockaddr_in6 *)(dst);
        PCHK(check = inet_pton(AF_INET6, ip, &ipV6addr->sin6_addr));
        if (check == 0) {
            fprintf(stderr, "invalide addrese: %s\n", ip);
            exit(EXIT_FAILURE);
        }
        ipV6addr->sin6_port = htons(port);
        ipV6addr->sin6_family = AF_INET6;
    } else {
        ipV4addr->sin_port = htons(port);
        ipV4addr->sin_family = AF_INET;
    }
}

struct addr_with_flag *parse_conf(const char *file_name) {
    struct addr_with_flag *res;
    size_t alloc_mem;
    
    alloc_mem = TABSIZE * sizeof(struct addr_with_flag);
    MCHK(res = malloc(alloc_mem));

    FILE *file;

    MCHK(file = fopen(file_name, "r"));

    char ip[100];
    int port;

    size_t i;
    for (i = 0; fscanf(file, "%[^|- ] | %d\n", ip, &port) != EOF; i++) {
        if (i >= alloc_mem) {
            alloc_mem *= 3;
            MCHK(res = realloc(res, alloc_mem));
        }
        convert(ip, port, &res[i].addr);
        res[i].end = false;
        res[i].ignore = false;
    }
    res[i + 1].end = true;

    PCHK(fclose(file));
    return res;
}
