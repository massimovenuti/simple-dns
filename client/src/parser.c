#include "parser.h"

struct type_addr convert(char ip[], int port) {
    struct type_addr res;
    int chek = inet_pton(AF_INET, ip, &((struct sockaddr_in *)&res.addr)->sin_addr);
    if (chek == -1) {
        perror("Error inet_pton_V4");
        exit(EXIT_FAILURE);
    } else if (chek == 0) {
        if ((chek = inet_pton(AF_INET6, ip, &((struct sockaddr_in6 *)&res.addr)->sin6_addr)) == -1) {
            perror("Error inet_pton_V6");
            exit(EXIT_FAILURE);
        } else if (chek == 0) {
            fprintf(stderr, "invalide addrese: %s\n", ip);
            exit(EXIT_FAILURE);
        }

        ((struct sockaddr_in6 *)&res.addr)->sin6_port = htons(port);
        res.type = AF_INET6;
    } else {
        ((struct sockaddr_in *)&res.addr)->sin_port = htons(port);
        res.type = AF_INET;
    }

    res.ignore = false;
    res.end = false;

    return res;
}

struct type_addr *parse(const char *file_name) {
    struct type_addr *res = malloc(TABSIZE * sizeof(struct type_addr));
    size_t alloc_mem = TABSIZE * sizeof(struct type_addr);

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

        res[i] = convert(ip, port);
    }
    res[i + 1].end = true;

    if (fclose(file) == EOF) {
        perror("Error fclose");
        exit(EXIT_FAILURE);
    }

    return res;
}
