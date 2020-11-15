#include "main.h"

void monitor_timeout(char *req, struct sockaddr_in6 addr, bool first_timeout) {
    if (first_timeout) {
        fprintf(stderr, YELLOW "%s" NEWLINE, req);
        afprint(stderr, addr);
        fprintf(stderr, " Suspicious");
    } else {
        fprintf(stderr, RED "%s" NEWLINE, req);
        afprint(stderr, addr);
        fprintf(stderr, " Timeout");
    }
    fprintf(stderr, RESET NEWLINE);
}

void monitor_reply(lserv *monitored, struct res res, struct sockaddr_in6 addr) {
    lserv l_serv = lssearch(*monitored, addr);
    if (!lsempty(l_serv)) {
        add_reply(&l_serv->server, res.time);
    } else {
        struct server s_serv = new_server(addr);
        add_reply(&s_serv, res.time);
        *monitored = lsadd(*monitored, s_serv);
    }
    fprintf(stderr, MAGENTA "res  %s" NEWLINE, res.req_name);
    fprintf(stderr, "time %fs" RESET NEWLINE, get_timevalue(res.time));
}

void monitor_shipment(lserv *monitored, char *req, struct sockaddr_in6 addr) {
    lserv l_serv = lssearch(*monitored, addr);
    if (!lsempty(l_serv)) {
        add_shipment(&l_serv->server);
    } else {
        struct server s_serv = new_server(addr);
        add_shipment(&s_serv);
        *monitored = lsadd(*monitored, s_serv);
    }
    fprintf(stderr, BLUE "req  %s\n", req);
    fprintf(stderr, "to   ");
    afprint(stderr, addr);
    fprintf(stderr, RESET NEWLINE);
}

bool handle_timeout(int soc, struct req *req, struct sockaddr_in6 addr,
                    lreq *reqs, lserv *ignored, lserv *suspicious,
                    lserv *monitored, bool monitoring) {
    struct server s_serv;
    lserv l_serv = lssearch(*monitored, addr);
    bool first_timeout = true;

    if (lsempty(l_serv)) {
        s_serv = new_server(addr);
    } else {
        s_serv = l_serv->server;
    }

    if (!lsbelong(addr, *ignored)) {
        if (!lsbelong(addr, *suspicious)) {
            *suspicious = lsadd(*suspicious, s_serv);
        } else {
            *ignored = lsadd(*ignored, s_serv);
            *suspicious = lsrm(*suspicious, addr);
            first_timeout = false;
        }
        if (monitoring) {
            monitor_timeout(req->name, addr, first_timeout);
        }
    }

    req->index = get_index(*reqs, *req);

    return send_request(soc, req, *ignored, monitored, monitoring);
}

void check_timeout(int soc, struct timeval *reset_t, lreq *reqs,
                   lserv *suspicious, lserv *ignored, lserv *monitored,
                   bool monitoring) {
    if (timeout(*reset_t, RESET_TIME)) {
        *ignored = lsrmh(*ignored);
        *reset_t = new_cooldown(RESET_TIME, 0);
    }
    for (lreq tmp = *reqs; !lrempty(tmp);
         tmp = (lrempty(tmp)) ? tmp : tmp->next) {
        int i = tmp->req.index % tmp->req.dest_addrs.len;
        struct sockaddr_in6 addr = tmp->req.dest_addrs.addrs[i];
        if (timeout(tmp->req.t, TIMEOUT)) {
            if (!handle_timeout(soc, &tmp->req, addr, reqs, ignored, suspicious,
                                monitored, monitoring)) {
                fprintf(stdout,
                        NEWLINE BOLDRED "%s Timeout" RESET NEWLINE NEWLINE,
                        tmp->req.name);
                *reqs = lrrm(*reqs, tmp->req.id);
                tmp = *reqs;
            }
        }
    }
}

void send_ack(int soc, int id, struct sockaddr_in6 addr) {
    char ack[20];
    int ack_len;
    if ((ack_len = snprintf(ack, 20, "ack|%d", id)) > 20 - 1) {
        fprintf(stderr, "ack too long");
        exit(EXIT_FAILURE);
    }
    PCHK(sendto(soc, ack, ack_len + 1, 0, (struct sockaddr *)&addr,
                (socklen_t)sizeof(struct sockaddr_in6)));
}

struct res receive_reply(int soc, lserv *monitored, bool monitoring) {
    char str_res[REQLEN];
    struct sockaddr_in6 src_addr;
    socklen_t len_addr = sizeof(struct sockaddr_in6);
    ssize_t len_res;
    PCHK(len_res = recvfrom(soc, str_res, REQLEN, 0,
                            (struct sockaddr *)&src_addr, &len_addr));
    struct res s_res = parse_res(str_res);

    if (s_res.id != -1) {
        send_ack(soc, s_res.id, src_addr);
    }

    if (monitoring) {
        monitor_reply(monitored, s_res, src_addr);
    }
    return s_res;
}

void handle_reply(int soc, int *id, lreq *reqs, struct req *req, struct res res,
                  lserv ignored, lserv *monitored, bool monitoring) {
    if (!strcmp(res.req_name, res.name)) {
        fprintf(stdout, NEWLINE BOLDGREEN "%s ", res.name);
        for (int i = 0; i < res.addrs.len; i++) {
            afprint(stdout, res.addrs.addrs[i]);
            fprintf(stdout, " ");
        }
        fprintf(stdout, RESET NEWLINE NEWLINE);
        *reqs = lrrm(*reqs, req->id);
    } else {
        update_req(reqs, req, *id, res.addrs);
        if (send_request(soc, req, ignored, monitored, monitoring)) {
            *id += 1;
        }
    }
}

int reqtostr(struct req s_req, char *str_req) {
    int len;
    if ((len = snprintf(str_req, REQLEN, "%d|%ld,%ld|%s", s_req.id,
                        s_req.t.tv_sec, s_req.t.tv_usec, s_req.name)) >
        REQLEN - 1) {
        fprintf(stderr, "Request too long" NEWLINE);
    }
    return len;
}

bool send_request(int soc, struct req *s_req, lserv ignored, lserv *monitored,
                  bool monitoring) {
    char str_req[REQLEN];
    PCHK(gettimeofday(&s_req->t, NULL));
    int req_len = reqtostr(*s_req, str_req);
    bool sent = false;
    int i = s_req->index % s_req->dest_addrs.len;
    for (int n = 0; n < s_req->dest_addrs.len && !sent; n++) {
        if (!lsbelong(s_req->dest_addrs.addrs[i], ignored)) {
            PCHK(sendto(soc, str_req, req_len + 1, 0,
                        (struct sockaddr *)&s_req->dest_addrs.addrs[i],
                        (socklen_t)sizeof(struct sockaddr_in6)));
            if (monitoring) {
                monitor_shipment(monitored, str_req,
                                 s_req->dest_addrs.addrs[i]);
            }
            s_req->index += n;
            sent = true;
        }
        i = (i + 1) % s_req->dest_addrs.len;
    }

    return sent;
}

void handle_request(char *input, int soc, int *id, lreq *reqs,
                    struct tab_addrs roots, lserv ignored, lserv *monitored,
                    bool monitoring) {
    struct req *req = new_req(reqs, *id, input, roots);
    if (send_request(soc, req, ignored, monitored, monitoring)) {
        *id += 1;
    } else {
        lsfree(ignored); /* /!\ */
        ignored = lsnew(); /* /!\ */
    }
}

void print_help() {
    fprintf(stderr, "usage : !stop" NEWLINE);
    /* /!\ à compléter */
}

void handle_command(char *command, int soc, int *id, lreq *reqs,
                    struct tab_addrs *roots, lserv ignored, lserv suspicious,
                    lserv *monitored, bool *goon, bool *monitoring) {
    char path[NAMELEN];
    if (!strcmp(command, "!stop")) {
        *goon = false;
    } else if (!strcmp(command, "!ignored")) {
        lsfprint(stderr, ignored);
    } else if (sscanf(command, "!loadroot %s\n", path)) { /* /!\ */
        *roots = parse_conf(path);
    } else if (!strcmp(command, "!help")) {
        print_help();
    } else if (sscanf(command, "!loadconf %s\n", path) == 1) { /* /!\ */
        load_reqfile(path, soc, id, reqs, roots, ignored, suspicious, monitored,
                     goon, monitoring);
    } else if (!strcmp(command, "!reset")) {
        lsfree(ignored); /* /!\ */
        lsfree(*monitored);
        lsfree(suspicious); /* /!\ */
        ignored = lsnew(); /* /!\ */
        *monitored = lsnew();
        suspicious = lsnew(); /* /!\ */
    } else if (!strcmp(command, "!monitoring")) {
        *monitoring = !*monitoring;
        if (*monitoring) {
            fprintf(stderr, "monitoring: enabled\n");
        } else {
            fprintf(stderr, "monitoring: disabled\n");
            lsfree(*monitored);
        }
    } else if (!strcmp(command, "!status")) {
        fprintf(stderr, NEWLINE "%d ignored servers" NEWLINE, lslen(ignored));
        lsfprint(stderr, ignored);
        if (*monitoring) {
            fprintf(stderr, NEWLINE "%d servers information" NEWLINE,
                    lslen(*monitored));
            lsfprint(stderr, *monitored);
        } else {
            fprintf(stderr, NEWLINE
                    "No information about servers. See \'!monitoring\'." NEWLINE
                        NEWLINE);
        }
    } else {
        fprintf(stderr, "!: invalid command \'%s\'" NEWLINE, command + 1);
        print_help();
    }
}

void read_input(FILE *stream, int soc, int *id, lreq *reqs,
                struct tab_addrs *roots, lserv ignored, lserv suspicious,
                lserv *monitored, bool *goon, bool *monitoring) {
    char input[NAMELEN];
    if (fscanf(stream, "%s", input) == EOF) {
        if (ferror(stream)) {
            perror("fscanf(req_file,\"%s\", name)");
            exit(EXIT_FAILURE);
        }
    }
    if (*input != '!') {
        handle_request(input, soc, id, reqs, *roots, ignored, monitored,
                       *monitoring);
    } else {
        handle_command(input, soc, id, reqs, roots, ignored, suspicious,
                       monitored, goon, monitoring);
    }
}

void read_network(int soc, int *id, lreq *reqs, lserv ignored, lserv *monitored,
                  bool monitoring) {
    struct res res = receive_reply(soc, monitored, monitoring);
    if (res.id == -1) {
        return;
    }
    lreq active_req = lrsearch(*reqs, res.id);
    if (lrempty(active_req)) {
        return;
    }
    if (res.code > 0) {
        handle_reply(soc, id, reqs, &active_req->req, res, ignored, monitored,
                     monitoring);
    } else {
        fprintf(stdout, BOLDRED "%s Not found" RESET NEWLINE NEWLINE,
                active_req->req.name);
        *reqs = lrrm(*reqs, res.id);
    }
}

void load_reqfile(const char *path, int soc, int *id, lreq *reqs,
                  struct tab_addrs *roots, lserv ignored, lserv suspicious,
                  lserv *monitored, bool *goon, bool *monitoring) {
    FILE *req_file;
    MCHK(req_file = fopen(path, "r"));
    while (!feof(req_file)) {
        read_input(req_file, soc, id, reqs, roots, ignored, suspicious,
                   monitored, goon, monitoring);
    }
    fclose(req_file);
}

int main(int argc, char const *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <config file path> [<reqs path>]" NEWLINE,
                argv[0]);
        exit(EXIT_FAILURE);
    }

    int soc = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
    struct tab_addrs roots = parse_conf(argv[1]);
    lreq reqs = lrnew();
    lserv ignored = lsnew(), suspicious = lsnew(), monitored = lsnew();
    struct timeval reset_t = new_cooldown(RESET_TIME, 0);
    bool monitoring = false, goon = true;
    int id = 0;

    if (argc == 3) {
        load_reqfile(argv[2], soc, &id, &reqs, &roots, ignored, suspicious,
                     &monitored, &goon, &monitoring);
    }

    fd_set ensemble;
    struct timeval timeout_loop;
    while (goon) {
        timeout_loop = new_timeval(1, 0);
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);
        PCHK(select(soc + 1, &ensemble, NULL, NULL, &timeout_loop));
        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            read_input(stdin, soc, &id, &reqs, &roots, ignored, suspicious,
                       &monitored, &goon, &monitoring);
        }
        if (FD_ISSET(soc, &ensemble)) {
            read_network(soc, &id, &reqs, ignored, &monitored, monitoring);
        }
        check_timeout(soc, &reset_t, &reqs, &suspicious, &ignored, &monitored,
                      monitoring);
    }

    lrfree(reqs);
    lsfree(ignored);
    lsfree(suspicious);
    lsfree(monitored);
    exit(EXIT_SUCCESS);
}
