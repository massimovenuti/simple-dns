#include "parser.h"

void convert(char ip[], int port, struct sockaddr *dst) {
    int chek;
    struct sockaddr_in *ipV4addr = (struct sockaddr_in *)(&dst);
    if ((chek = inet_pton(AF_INET, ip, &ipV4addr->sin_addr)) == -1) {
        perror("Error inet_pton_V4");
        exit(EXIT_FAILURE);
    } else if (chek == 0) {
        struct sockaddr_in6 *ipV6addr = (struct sockaddr_in6 *)(&dst);
        if ((chek = inet_pton(AF_INET6, ip, &ipV6addr->sin6_addr)) == -1) {
            perror("Error inet_pton_V6");
            exit(EXIT_FAILURE);
        } else if (chek == 0) {
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

struct addr_with_flag *parse(const char *file_name) {
    struct addr_with_flag *res = malloc(TABSIZE * sizeof(struct addr_with_flag));
    size_t alloc_mem = TABSIZE * sizeof(struct addr_with_flag);

    FILE *file;

    if ((file = fopen(file_name, "r")) == NULL) {
        perror("Error fopen");
        exit(EXIT_FAILURE);
    }

    char ip[100];
    int port;

    size_t i;
    for (i = 0; fscanf(file, "%[^|]|%d\n", ip, &port) != EOF; i++) {
        if (i >= alloc_mem) {
            res = realloc(res, alloc_mem * 3);
        }

        convert(ip, port, &res[i].addr);
        res[i].end = false;
        res[i].ignore = false;
    }
    res[i + 1].end = true;

    if (fclose(file) == EOF) {
        perror("Error fclose");
        exit(EXIT_FAILURE);
    }

    return res;
}
