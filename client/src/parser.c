#include "parser.h"

struct sockaddr_in6 convert(char ip[], int port) {
    int check;
    struct sockaddr_in6 res;
    struct sockaddr_in *ipV4addr = (struct sockaddr_in *)(&res);
    PCHK(check = inet_pton(AF_INET, ip, &ipV4addr->sin_addr));

    if (check == 0) {
        PCHK(check = inet_pton(AF_INET6, ip, &res.sin6_addr));
        if (check == 0) {
            fprintf(stderr, "Invalid address: %s\n", ip);
            exit(EXIT_FAILURE);
        }
        res.sin6_port = htons(port);
        res.sin6_family = AF_INET6;
    } else {
        ipV4addr->sin_port = htons(port);
        ipV4addr->sin_family = AF_INET;
    }
    return res;
}

struct tab_addrs parse_conf(const char *file_name) {
    struct tab_addrs res;
    FILE *file;
    MCHK(file = fopen(file_name, "r"));
    char ip[IPLEN];
    int port;
    int i;
    for (i = 0; fscanf(file, "%[^|- ] | %d\n", ip, &port) != EOF; i++) {
        if (i >= TABSIZE) {
            // raler
            break;
        }
        res.addrs[i] = convert(ip, port);
    }
    res.len = i; /* /!\ peut être une erreur */
    PCHK(fclose(file));
    return res;
}

struct res parse_res(char *res) {
    struct res s_res;
    char tab_addrs[1024];  // à modifier
    *tab_addrs = '\0';

    if (sscanf(res, " %d | %ld,%ld | %[^|- ] | %d | %s", &s_res.id,
               &s_res.time.tv_sec, &s_res.time.tv_usec, s_res.req_name,
               &s_res.code, tab_addrs) < 4) {
        fprintf(stderr, "Incorrect server result\n");
        s_res.id = -1;
        return s_res;
    }

    struct timeval t;
    PCHK(gettimeofday(&t, NULL));

    s_res.time = op_timeval(t, '-', s_res.time);

    char *token;
    char name[NAMELEN];
    char ip[IPLEN];
    char port[PORTLEN];
    int nb_addrs = 0;
    bool first = true;

    token = strtok(tab_addrs, SEPARATOR);

    if (token == NULL) {
        s_res.addrs.len = 0;
    } else {
        do {
            if (sscanf(token, " %[^, ] , %[^, ] , %[^, ] ", name, ip, port) !=
                3) {
                fprintf(stderr, "Server result incorrect\n");
                exit(EXIT_FAILURE);
            }
            if (first) {
                MCHK(strcpy(s_res.name, name));
                first = false;
            }
            s_res.addrs.addrs[nb_addrs] = convert(ip, atoi(port));
            nb_addrs++;
            token = strtok(NULL, SEPARATOR);
        } while (nb_addrs < TABSIZE && token != NULL);
        s_res.addrs.len = nb_addrs; /* /!\ peut être une erreur */
    }
    return s_res;
}
