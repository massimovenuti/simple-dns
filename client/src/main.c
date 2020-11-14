#include "main.h"

bool is_ignored(laddr l, struct sockaddr_in6 addr) {
    return !laddr_empty(laddr_search(l, addr));
}

bool timeout(struct req req) {
    struct timeval cur_t;
    PCHK(gettimeofday(&cur_t, NULL));
    cur_t.tv_sec -= TIMEOUT;
    return timercmp(&cur_t, &req.t, >=);
}

void check_timeout(int soc, lreq *lr, laddr *suspicious, laddr *ignored,
                   laddr monitored, bool monitoring) {
    for (lreq tmp = *lr; !lreq_empty(tmp);
         tmp = (lreq_empty(tmp)) ? tmp : tmp->next) {
        if (timeout(tmp->req)) {
            struct sockaddr_in6 *addr =
                &tmp->req.dest_addrs
                     .addrs[tmp->req.index % tmp->req.dest_addrs.len];
            laddr moni = laddr_search(monitored, *addr);
            if (laddr_empty(laddr_search(*suspicious, *addr))) {
                if (!is_ignored(*ignored, *addr)) {
                    if (monitoring) {
                        fprintf(stderr, YELLOW "%s ", tmp->req.name);
                        fprint_addr(stderr, *addr);
                        fprintf(stderr, " SUSPICIOUS" RESET NEWLINE NEWLINE);
                    }
                    if (!laddr_empty(moni)) {
                        *suspicious = laddr_add(*suspicious, moni->m_addr);
                    } else {
                        *suspicious = laddr_add(*suspicious, new_maddr(*addr));
                    }
                }
                tmp->req.index = get_index(*lr, tmp->req);
                if (!send_req(soc, &tmp->req, *ignored, monitoring)) {
                    fprintf(stdout, BOLDRED "%s TIMEOUT" RESET NEWLINE NEWLINE,
                            tmp->req.name);
                    *lr = lreq_rm(*lr, tmp->req.id);
                    tmp = *lr;
                }
            } else {
                if (monitoring) {
                    fprintf(stderr, NEWLINE RED "%s ", tmp->req.name);
                    fprint_addr(stderr, *addr);
                    fprintf(stderr, RED " TIMEOUT" RESET NEWLINE NEWLINE);
                }
                laddr moni = laddr_search(monitored, *addr);
                if (!laddr_empty(moni)) {
                    *ignored = laddr_add(*ignored, moni->m_addr);
                } else {
                    *ignored = laddr_add(*ignored, new_maddr(*addr));
                }
                *suspicious = laddr_rm(*suspicious, *addr);
                tmp->req.index = get_index(*lr, tmp->req);
                if (!send_req(soc, &tmp->req, *ignored, monitoring)) {
                    fprintf(stdout, BOLDRED "%s TIMEOUT" RESET NEWLINE NEWLINE,
                            tmp->req.name);
                    *lr = lreq_rm(*lr, tmp->req.id);
                    tmp = *lr;
                }
            }
        }
    }
}

bool send_req(int soc, struct req *req, laddr ignored, bool monitoring) {
    char req_char[REQLEN];
    int len_req;
    PCHK(gettimeofday(&req->t, NULL));
    if ((len_req = snprintf(req_char, REQLEN, "%d|%ld,%ld|%s", req->id,
                            req->t.tv_sec, req->t.tv_usec, req->name)) >
        REQLEN - 1) {
        fprintf(stderr, "Request too long");
    }
    if (monitoring) {
        fprintf(stderr, BLUE "req  %s\n", req_char);
    }
    bool req_send = false;
    int i = req->index % req->dest_addrs.len;
    for (int n = 0; n < req->dest_addrs.len && !req_send; n++) {
        if (!is_ignored(ignored, req->dest_addrs.addrs[i])) {
            if (monitoring) {
                fprintf(stderr, "to   ");
                fprint_addr(stderr, req->dest_addrs.addrs[i]);
                fprintf(stderr, RESET NEWLINE);
            }
            PCHK(sendto(soc, req_char, len_req + 1, 0,
                        (struct sockaddr *)&req->dest_addrs.addrs[i],
                        (socklen_t)sizeof(struct sockaddr_in6)));
            req->index += n;
            req_send = true;
        }
        i = (i + 1) % req->dest_addrs.len;
    }
    return req_send;
}

struct res receive_res(int soc, laddr *monitored, bool monitoring) {
    char res[REQLEN];
    struct sockaddr_in6 src_addr;
    socklen_t len_addr = sizeof(struct sockaddr_in6);
    ssize_t len_res;
    PCHK(len_res = recvfrom(soc, res, REQLEN, 0, (struct sockaddr *)&src_addr,
                            &len_addr));
    struct res struc_res = parse_res(res);
    if (monitoring) {
        laddr tmp = laddr_search(*monitored, src_addr);
        if (!laddr_empty(tmp)) {
            use(&tmp->m_addr, struc_res.time);
        } else {
            struct monitored_addr maddr = new_maddr(src_addr);
            use(&maddr, struc_res.time);
            *monitored = laddr_add(*monitored, maddr);
        }
        fprintf(stderr, MAGENTA "res  %s" NEWLINE, res);
        fprintf(stderr, "time %fs" RESET NEWLINE,
                get_timevalue(struc_res.time));
    }
    return struc_res;
}

void print_res(char *req, struct tab_addrs ta) {
    fprintf(stdout, NEWLINE BOLDGREEN "%s ", req);
    for (int i = 0; i < ta.len; i++) {
        fprint_addr(stdout, ta.addrs[i]);
        fprintf(stdout, " ");
    }
    printf(RESET NEWLINE NEWLINE);
}

void read_network(int soc, int *id, lreq *reqs, laddr ignored, laddr *monitored,
                  bool monitoring) {
    struct res res = receive_res(soc, monitored, monitoring);
    lreq active_req = lreq_search(*reqs, res.id);
    if (lreq_empty(active_req)) {
        return;
    }
    if (res.code > 0) {
        if (!strcmp(res.req_name, res.name)) {
            print_res(res.req_name, res.addrs);
            *reqs = lreq_rm(*reqs, res.id);
        } else {
            update_req(reqs, &active_req->req, *id, res.addrs);
            if (send_req(soc, &active_req->req, ignored, monitoring)) {
                *id += 1;
            } else {
                *reqs = lreq_rm(*reqs, active_req->req.id);
            }
        }
    } else {
        printf(NEWLINE BOLDRED "%s NOT FOUND" RESET NEWLINE NEWLINE,
               active_req->req.name);
        *reqs = lreq_rm(*reqs, res.id);
    }
}

void handle_request(char *input, int soc, int *id, lreq *lr,
                    struct tab_addrs roots, laddr ignored, bool monitoring) {
    struct req *req = new_req(lr, *id, input, roots);
    if (send_req(soc, req, ignored, monitoring)) {
        *id += 1;
    } else {
        *lr = lreq_rm(*lr, req->id);
    }
}

void handle_monitoring(laddr *monitored, bool *monitoring) {
    *monitoring = !*monitoring;
    if (monitoring) {
        fprintf(stderr, "monitoring: enabled\n");
    } else {
        fprintf(stderr, "monitoring: disabled\n");
        laddr_destroy(*monitored);
    }
}

void handle_ignored(laddr ignored) {
    if (laddr_len(ignored) > 0) {
        fprintf(stderr, "ignored server:" NEWLINE);
        laddr_fprint(stderr, ignored);
    } else {
        fprintf(stderr, "no server ignored." NEWLINE);
    }
}

void handle_status(laddr ignored, laddr suspicious, laddr monitored,
                   bool monitoring) {
    fprintf(stderr, "%d servers ignored" NEWLINE, laddr_len(ignored));
    laddr_fprint(stderr, ignored);
    fprintf(stderr, NEWLINE "%d servers suspicious" NEWLINE,
            laddr_len(suspicious));
    laddr_fprint(stderr, suspicious);
    if (monitoring) {
        fprintf(stderr, NEWLINE "%d servers used" NEWLINE,
                laddr_len(monitored));
        laddr_fprint(
            stderr,
            monitored); /* /!\ faire une fonction laddr_print et addr_print */
    } else {
        fprintf(stderr, NEWLINE
                "No information about servers used. See \'!monitoring\'.");
    }
}

void handle_unknown(char *input) {
    fprintf(stderr, "!: invalid option \'%s\'" NEWLINE, input + 1);
    fprintf(stderr, "usage : !stop" NEWLINE); /* /!\ à compléter */
}

void handle_reqfile(const char *path, int soc, int *id, lreq *reqs,
                    struct tab_addrs *roots, laddr ignored, laddr suspicious,
                    laddr *monitored, bool *goon, bool *monitoring) {
    FILE *req_file;
    MCHK(req_file = fopen(path, "r"));
    while (!feof(req_file)) {
        read_input(req_file, soc, id, reqs, roots, ignored, suspicious,
                   monitored, goon, monitoring);
    }
    fclose(req_file);
}

void handle_command(char *command, int soc, int *id, lreq *reqs,
                    struct tab_addrs *roots, laddr ignored, laddr suspicious,
                    laddr *monitored, bool *goon, bool *monitoring) {
    char path[NAMELEN];
    if (!strcmp(command, "!stop")) {
        *goon = false;
    } else if (!strcmp(command, "!monitoring")) {
        handle_monitoring(monitored, monitoring);
    } else if (!strcmp(command, "!ignored")) {
        handle_ignored(ignored);
    } else if (!strcmp(command, "!status")) {
        handle_status(ignored, suspicious, *monitored, *monitoring);
    } else if (sscanf(command, "!load %s\n", path) == 1) {
        /* /!\ ne fonctionne pas */
        handle_reqfile(path, soc, id, reqs, roots, ignored, suspicious,
                       monitored, goon, monitoring);
    } else if (sscanf(command, "!root %s\n", path)) {
        /* /!\ ne fonctionne pas */
        *roots = parse_conf(path);
    } else {
        handle_unknown(command);
    }
}

void read_input(FILE *stream, int soc, int *id, lreq *reqs,
                struct tab_addrs *roots, laddr ignored, laddr suspicious,
                laddr *monitored, bool *goon, bool *monitoring) {
    char input[NAMELEN];
    if (fscanf(stream, "%s", input) == EOF) {
        if (ferror(stream)) {
            perror("fscanf(req_file,\"%s\", name)");
            exit(EXIT_FAILURE);
        }
    } else {
        if (*input == '!') {
            handle_command(input, soc, id, reqs, roots, ignored, suspicious,
                           monitored, goon, monitoring);
        } else {
            handle_request(input, soc, id, reqs, *roots, ignored, *monitoring);
        }
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <config file path> [<reqs path>]" NEWLINE,
                argv[0]);
        exit(EXIT_FAILURE);
    }

    int soc = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
    struct tab_addrs roots = parse_conf(argv[1]);
    lreq reqs = lreq_new();
    laddr ignored = laddr_new(), suspicious = laddr_new(),
          monitored = laddr_new();
    bool monitoring = false, goon = true;
    int id = 0;

    if (argc == 3) {
        handle_reqfile(argv[2], soc, &id, &reqs, &roots, ignored, suspicious,
                       &monitored, &goon, &monitoring);
    }

    struct timeval timeout_loop = new_timeval(1, 0);
    fd_set ensemble;
    while (goon) {
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
        check_timeout(soc, &reqs, &suspicious, &ignored, monitored, monitoring);
    }

    lreq_destroy(reqs);
    laddr_destroy(ignored);
    laddr_destroy(suspicious);
    laddr_destroy(monitored);
    exit(EXIT_SUCCESS);
}
