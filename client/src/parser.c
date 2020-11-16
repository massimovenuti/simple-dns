/**
 * @file parser.c
 * @author Massimo Venuti, Alexandre Vogel
 * @brief Parser côté client - fichier source
 * @date 2020-11-16
 * 
 */
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
    if (ferror(file)) {
        perror("fopen(file_name, \"r\")");
        exit(EXIT_FAILURE);
    }
    char ip[LIP];
    int port;
    int i;
    for (i = 0;
         i < TABSIZE && fscanf(file, "%" STR(LIP) "[^|- ] | %" STR(LPORT) "d\n",
                               ip, &port) != EOF;
         i++) {
        res.addrs[i] = convert(ip, port);
    }
    res.len = i;
    PCHK(fclose(file));
    return res;
}

void update_restime(struct res *res) {
    struct timeval t;
    PCHK(gettimeofday(&t, NULL));
    res->time = op_timeval(t, res->time, '-');
}

void parse_addrs(struct res *res, char *addrs) {
    char *token;
    char name[LNAME];
    char ip[LIP];
    char port[LPORT];

    token = strtok(addrs, SEPARATOR);

    int i;
    for (i = 0; i < TABSIZE && token != NULL; i++) {
        if (sscanf(token,
                   " %" STR(LNAME) "[^, ] , %" STR(LIP) "[^, ] , %" STR(
                       LPORT) "[^, ] ",
                   name, ip, port) != 3) {
            fprintf(stderr, "incorrect server result\n");
            exit(EXIT_FAILURE);
        }
        if (i == 0) {
            MCHK(strcpy(res->name, name));
        }
        res->addrs.addrs[i] = convert(ip, atoi(port));
        token = strtok(NULL, SEPARATOR);
    }
    res->addrs.len = i;
}

struct res parse_res(char *res) {
    struct res s_res;
    char addrs[LRES];
    *addrs = '\0';

    if (sscanf(res,
               " %" STR(LID) "d | %" STR(LTIME) "ld,%" STR(LTIME) "ld | %" STR(
                   LNAME) "[^|- ] | %" STR(LCODE) "d | "
                                                  "%" STR(LRES) "s",
               &s_res.id, &s_res.time.tv_sec, &s_res.time.tv_usec,
               s_res.req_name, &s_res.code, addrs) < 4) {
        fprintf(stderr, "Incorrect server result\n");
        s_res.id = -1;
        return s_res;
    }

    parse_addrs(&s_res, addrs);
    update_restime(&s_res);
    return s_res;
}
