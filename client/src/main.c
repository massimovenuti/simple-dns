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

void check_timeout(int soc, lreq lr, laddr suspicious, laddr ignored, struct running run) {
    for (lreq tmp = lr; !lreq_empty(tmp); tmp = tmp->next) {
        if (timeout(tmp->req)) {
            struct sockaddr_in6 *addr = &tmp->req.dest_addrs.addrs[lr->req.index];
            laddr la = laddr_search(suspicious, *addr);
            if (!laddr_empty(la)) {
                suspicious = laddr_add(suspicious, la->m_addr);
                send_req(soc, &tmp->req, ignored, run);
            } else {
                fprintf(stderr, "%s:\nTIMEOUT", tmp->req.name);
                ignored = laddr_add(ignored, la->m_addr);
                suspicious = laddr_rm(suspicious, *addr);
                tmp->req.index = get_index(lr, tmp->req);
                send_req(soc, &tmp->req, ignored, run);
            }
        }
    }
}

void send_req(int soc, struct req *req, laddr ignored, struct running run) {
    char req_char[REQLEN];
    int len_req;
    if ((len_req = snprintf(req_char, REQLEN, "%d|%ld,%ld|%s", req->id, req->t.tv_sec, req->t.tv_usec, req->name)) > REQLEN - 1) {
        fprintf(stderr, "Request too long");
    }
    if (run.monitoring) {
        fprintf(stderr, "req: %s\n", req_char);
    }
    bool sent = false;
    int i = req->index;
    for (int n = 0; n < req->dest_addrs.len && !sent; n++) {
        if (!is_ignored(ignored, req->dest_addrs.addrs[i])) {
            PCHK(sendto(soc, req_char, len_req + 1, 0, (struct sockaddr *)&req->dest_addrs.addrs[i], (socklen_t)sizeof(struct sockaddr_in6)));
            req->index = i;
            sent = true;
        }
        i = (i + 1) % req->dest_addrs.len;
    }
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
            struct req req = new_req(reqs, *id, input, root_addr);
            *id += 1;
            send_req(soc, &req, ignored, *run);
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
                    fprintf(stderr, "%d servers used\n" , laddr_len(*monitored));
                    laddr_fprint(stderr, *monitored);               /* /!\ faire une fonction laddr_print et addr_print */
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
            *reqs = lreq_rm(*reqs, res.id);
        } else {
            update_req(reqs, &active_req->req, *id, res.addrs);
            send_req(soc, &active_req->req, ignored, run);
            *id += 1;
        }
    } else {
        printf("%s:\n", active_req->req.name);
        printf("\33[1;31mNot found\033[0m\n");
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
            check_timeout(soc, reqs, suspicious, ignored, run);
        }
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
    }

    if (!run.interactive) {
        fclose(req_file);
    }

    laddr_destroy(ignored);
    laddr_destroy(suspicious);
    lreq_destroy(reqs);
    exit(EXIT_SUCCESS);
}
