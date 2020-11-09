#include "main.h"

bool is_ignored(laddr l, struct sockaddr_in6 addr) {
    return !laddr_empty(laddr_search(l, addr));
}

void send_req(int soc, struct req *req, laddr ignored, laddr *used, bool monitoring) {
    char req_char[REQLEN];

    struct timeval t;
    PCHK(gettimeofday(&t, NULL));

    int len_req;
    if ((len_req = snprintf(req_char, REQLEN, "%d|%ld,%ld|%s", req->id, t.tv_sec, t.tv_usec, req->name)) > REQLEN - 1)
        fprintf(stderr, "Request too long");

    if (monitoring)
        fprintf(stderr, "req: %s\n", req_char);

    bool send = false;
    int i = req->index;
    for (int n = 0; n < req->dest_addrs.len && !send; n++) {
        if (!is_ignored(ignored, req->dest_addrs.addrs[i])) {  //&& minimal_use()) {
            PCHK(sendto(soc, req_char, len_req + 1, 0, (struct sockaddr *)&req->dest_addrs.addrs[i], (socklen_t)sizeof(struct sockaddr_in6)));
            req->index = i;
            *used = laddr_add(*used, req->dest_addrs.addrs[i]);
            send = true;
        }
        i = (i + 1) % req->dest_addrs.len;
    }
}

struct res receive_res(int soc, laddr ignored, laddr *used, bool monitoring) {
    char res[REQLEN];
    (void)ignored;

    struct sockaddr_in6 src_addr;
    socklen_t len_addr = sizeof(struct sockaddr_in6);
    ssize_t len_res;

    PCHK(len_res = recvfrom(soc, res, REQLEN, 0, (struct sockaddr *)&src_addr, &len_addr));
    *used = laddr_rm(*used, src_addr);
    struct res struc_res = parse_res(res);

    if (monitoring) {
        fprintf(stderr, "res: %s\n", res);
        fprintf(stderr, "in: %lds %ldms\n\n", struc_res.time.tv_sec, struc_res.time.tv_usec / 1000);
    }

    return struc_res;
}

void read_input(FILE *stream, int soc, int *id, lreq *reqs, struct tab_addrs root_addr, laddr ignored, laddr *used, bool *goon, bool *monitoring) {
    (void)goon;
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
            send_req(soc, &req, ignored, used, *monitoring);
        } else {
            if (!strcmp(input, "!stop")) {
                *goon = false;
            } else if (!strcmp(input, "!monitoring")) {
                *monitoring = !*monitoring;
                if (monitoring) {
                    fprintf(stderr, "monitoring:enabel");
                } else {
                    fprintf(stderr, "monitoring:disabel");
                }
            } else if (!strcmp(input, "!ignored")) {
                fprintf(stderr, "ignored server:\n");
                laddr_fprint(stderr, ignored);
            }
        }
    }
}

void read_network(int soc, int *id, lreq *reqs, laddr ignored, laddr *used, bool monitoring) {
    struct res res = receive_res(soc, ignored, used, monitoring);
    lreq active_req = lreq_search(*reqs, res.id);
    if (lreq_empty(active_req)) {
        return;
    }
    active_req->req.dest_addrs = res.addrs;

    if (res.code > 0) {
        if (!strcmp(res.req_name, res.name)) {
            printf("%s:\n", res.name);
            for (int i = 0; i < res.addrs.len; i++) {
                fprint_addr(stdout, res.addrs.addrs[i]);
            }
            printf("\n");
            *reqs = lreq_rm(*reqs, res.id);
        } else {
            active_req->req.id = *id;
            active_req->req.index = 0;
            send_req(soc, &active_req->req, ignored, used, monitoring);
            *id += 1;
        }
    } else {
        printf("%s:\n", active_req->req.name);
        printf("\33[1;31mNot found\033[0m\n");
        printf("\n");
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
    laddr ignored = laddr_new(), used = laddr_new();
    lreq reqs = lreq_new();

    bool monitoring = false;
    bool goon = true;
    bool interactif = true;

    int id = 0;
    fd_set ensemble;
    struct timeval timeout_loop = {1, 0};

    root_addr = parse_conf(argv[1]);

    PCHK(soc = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP));

    if (argc == 3) {
        interactif = false;
        MCHK(req_file = fopen(argv[2], "r"));
    }

    while (goon && interactif) {
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);
        PCHK(select(soc + 1, &ensemble, NULL, NULL, &timeout_loop));

        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
        } else if (FD_ISSET(soc, &ensemble)) {
            read_network(soc, &id, &reqs, ignored, &used, monitoring);
        } else {
        }
    }

    while (goon && !interactif) {
        while (!feof(req_file)) {
            read_input(req_file, soc, &id, &reqs, root_addr, ignored, &used, &goon, &monitoring);
        }

        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);
        PCHK(select(soc + 1, &ensemble, NULL, NULL, &timeout_loop));

        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            read_input(stdin, soc, &id, &reqs, root_addr, ignored, &used, &goon, &monitoring);
        } else if (FD_ISSET(soc, &ensemble)) {
            read_network(soc, &id, &reqs, ignored, &used, monitoring);
        } else {
        }
    }

    if (!interactif) {
        fclose(req_file);
    }

    laddr_destroy(ignored);
    laddr_destroy(used);
    lreq_destroy(reqs);
    exit(EXIT_SUCCESS);
}
