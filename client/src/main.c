#include "main.h"

bool is_ignored(laddr l, struct sockaddr_in6 addr) {
    return !laddr_empty(laddr_search(l, addr));
}

bool timeout(struct req r) {
    struct timeval cur_t;
    PCHK(gettimeofday(&cur_t, NULL));
    cur_t.tv_sec -= TIMEOUT;
    return timercmp(&cur_t, &r.t, >=);
}

void check_timeout(int soc, lreq *lr, laddr *suspicious, laddr *ignored, laddr monitored, struct running run) {
    for (lreq tmp = *lr; !lreq_empty(tmp); tmp = (lreq_empty(tmp)) ? tmp : tmp->next) {
        if (timeout(tmp->req)) {
            struct sockaddr_in6 *addr = &tmp->req.dest_addrs.addrs[tmp->req.index % tmp->req.dest_addrs.len];
            laddr moni = laddr_search(monitored, *addr);
            if (laddr_empty(laddr_search(*suspicious, *addr))) {
                if (!is_ignored(*ignored, *addr)) {
                    if (run.monitoring) {
                        fprintf(stderr, "%s:\n", tmp->req.name);
                        fprint_addr(stderr, *addr);
                        fprintf(stderr, "Suspicious\n\n");
                    }
                    if (!laddr_empty(moni)) {
                        *suspicious = laddr_add(*suspicious, moni->m_addr);
                    } else {
                        *suspicious = laddr_add(*suspicious, new_maddr(*addr));
                    }
                }
                tmp->req.index = get_index(*lr, tmp->req);
                if (!send_req(soc, &tmp->req, *ignored, run)) {
                    *lr = lreq_rm(*lr, tmp->req.id);
                    tmp = *lr;
                }
            } else {
                if (run.monitoring) {
                    fprintf(stderr, "%s:\n", tmp->req.name);
                    fprint_addr(stderr, *addr);
                    fprintf(stderr, "TIMEOUT\n\n");
                }
                laddr moni = laddr_search(monitored, *addr);
                if (!laddr_empty(moni)) {
                    *ignored = laddr_add(*ignored, moni->m_addr);
                } else {
                    *ignored = laddr_add(*ignored, new_maddr(*addr));
                }
                *suspicious = laddr_rm(*suspicious, *addr);
                tmp->req.index = get_index(*lr, tmp->req);
                if (!send_req(soc, &tmp->req, *ignored, run)) {
                    *lr = lreq_rm(*lr, tmp->req.id);
                    tmp = *lr;
                }
            }
        }
    }
}

bool send_req(int soc, struct req *req, laddr ignored, struct running run) {
    char req_char[REQLEN];
    int len_req;
    PCHK(gettimeofday(&req->t, NULL));
    if ((len_req = snprintf(req_char, REQLEN, "%d|%ld,%ld|%s", req->id, req->t.tv_sec, req->t.tv_usec, req->name)) > REQLEN - 1) {
        fprintf(stderr, "Request too long");
    }
    if (run.monitoring) {
        fprintf(stderr, "req: %s\n", req_char);
    }
    bool req_send = false;
    int i = req->index % req->dest_addrs.len;
    for (int n = 0; n < req->dest_addrs.len && !req_send; n++) {
        if (!is_ignored(ignored, req->dest_addrs.addrs[i])) {
            if (run.monitoring) {
                fprint_addr(stderr, req->dest_addrs.addrs[i]);
                printf("\n");
            }
            PCHK(sendto(soc, req_char, len_req + 1, 0, (struct sockaddr *)&req->dest_addrs.addrs[i], (socklen_t)sizeof(struct sockaddr_in6)));
            req->index += n;
            req_send = true;
        }
        i = (i + 1) % req->dest_addrs.len;
    }
    return req_send;
}

struct res receive_res(int soc, laddr *monitored, struct running run) {
    char res[REQLEN];
    struct sockaddr_in6 src_addr;
    socklen_t len_addr = sizeof(struct sockaddr_in6);
    ssize_t len_res;
    PCHK(len_res = recvfrom(soc, res, REQLEN, 0, (struct sockaddr *)&src_addr, &len_addr));
    struct res struc_res = parse_res(res);
    if (run.monitoring) {
        laddr tmp = laddr_search(*monitored, src_addr);
        if (!laddr_empty(tmp)) {
            use(&tmp->m_addr, struc_res.time);
        } else {
            struct monitored_addr maddr = new_maddr(src_addr);
            use(&maddr, struc_res.time);
            *monitored = laddr_add(*monitored, maddr);
        }
        fprintf(stderr, "res: %s\n", res);
        fprintf(stderr, "in: %lds %ldms\n\n", struc_res.time.tv_sec, struc_res.time.tv_usec / 1000);
    }
    return struc_res;
}

void read_input(FILE *stream, int soc, int *id, lreq *reqs, struct tab_addrs root_addr, laddr ignored, laddr suspicious, laddr *monitored, struct running *run) {
    char input[NAMELEN];
    if (fscanf(stream, "%s", input) == EOF) {
        if (ferror(stream)) {
            perror("fscanf(req_file,\"%s\", name)");
            exit(EXIT_FAILURE);
        }
    } else {
        if (*input != '!') {
            struct req *req = new_req(reqs, *id, input, root_addr);
            if (!send_req(soc, req, ignored, *run)) {
                *reqs = lreq_rm(*reqs, req->id);
            } else {
                *id += 1;
            }
        } else {
            if (!strcmp(input, "!stop")) {
                run->goon = false;
            } else if (!strcmp(input, "!monitoring")) {
                run->monitoring = !run->monitoring;
                if (run->monitoring) {
                    fprintf(stderr, "monitoring: enabled\n");
                } else {
                    fprintf(stderr, "monitoring: disabled\n");
                    laddr_destroy(*monitored);
                }
            } else if (!strcmp(input, "!ignored")) {
                fprintf(stderr, "ignored server:\n");
                laddr_fprint(stderr, ignored);
            } else if (!strcmp(input, "!status")) {
                fprintf(stderr, "%d SERVERS IGNORED\n", laddr_len(ignored));
                laddr_fprint(stderr, ignored);
                fprintf(stderr, "\n%d SERVERS TIMEOUT ONCE\n", laddr_len(suspicious));
                laddr_fprint(stderr, suspicious);
                if (run->monitoring) {
                    fprintf(stderr, "\nSERVERS INFORMATIONS\n");
                    fprintf(stderr, "%d servers used\n", laddr_len(*monitored));
                    laddr_fprint(stderr, *monitored); /* /!\ faire une fonction laddr_print et addr_print */
                }
            } else {
                fprintf(stderr, "!: invalid option \'%s\'\n", input + 1);
                fprintf(stderr, "usage : !stop\n"); /* /!\ à compléter */
            }
        }
    }
}

void read_network(int soc, int *id, lreq *reqs, laddr ignored, laddr *monitored, struct running run) {
    struct res res = receive_res(soc, monitored, run);
    lreq active_req = lreq_search(*reqs, res.id);
    if (lreq_empty(active_req)) {
        return;
    }
    if (res.code > 0) {
        if (!strcmp(res.req_name, res.name)) {
            printf("%s:\n", res.name);
            for (int i = 0; i < res.addrs.len; i++) {
                fprint_addr(stdout, res.addrs.addrs[i]);
            }
            printf("\n");
            *reqs = lreq_rm(*reqs, res.id);
        } else {
            update_req(reqs, &active_req->req, *id, res.addrs);
            if (!send_req(soc, &active_req->req, ignored, run)) {
                *reqs = lreq_rm(*reqs, active_req->req.id);
            } else {
                *id += 1;
            }
        }
    } else {
        printf("%s:\n\33[1;31mNot found\033[0m\n\n", active_req->req.name);
        *reqs = lreq_rm(*reqs, res.id);
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: <path of config file>");
        exit(EXIT_FAILURE);
    }

    FILE *req_file;

    int soc;

    struct tab_addrs root_addr;

    laddr ignored = laddr_new(),
          suspicious = laddr_new(),
          monitored = laddr_new();

    lreq reqs = lreq_new();

    struct running run = {true, false, true};

    int id = 0;
    fd_set ensemble;
    struct timeval timeout_loop = new_timeval(1, 0);

    root_addr = parse_conf(argv[1]);

    PCHK(soc = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP));

    if (argc == 3) {
        run.interactive = false;
        MCHK(req_file = fopen(argv[2], "r"));
    }

    while (run.goon && run.interactive) {
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);
        PCHK(select(soc + 1, &ensemble, NULL, NULL, &timeout_loop));

        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            read_input(stdin, soc, &id, &reqs, root_addr, ignored, suspicious, &monitored, &run);
        } else if (FD_ISSET(soc, &ensemble)) {
            read_network(soc, &id, &reqs, ignored, &monitored, run);
        } else {
            check_timeout(soc, &reqs, &suspicious, &ignored, monitored, run);
        }
        check_timeout(soc, &reqs, &suspicious, &ignored, monitored, run);
    }

    while (run.goon && !run.interactive) {
        while (!feof(req_file)) {
            read_input(req_file, soc, &id, &reqs, root_addr, ignored, suspicious, &monitored, &run);
        }

        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);
        PCHK(select(soc + 1, &ensemble, NULL, NULL, &timeout_loop));

        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            read_input(stdin, soc, &id, &reqs, root_addr, ignored, suspicious, &monitored, &run);
        } else if (FD_ISSET(soc, &ensemble)) {
            read_network(soc, &id, &reqs, ignored, &monitored, run);
        } else {
        }
        check_timeout(soc, &reqs, &suspicious, &ignored, monitored, run);
    }

    if (!run.interactive) {
        fclose(req_file);
    }

    laddr_destroy(ignored);
    laddr_destroy(suspicious);
    lreq_destroy(reqs);
    exit(EXIT_SUCCESS);
}
