#include "main.h"

void init_tab_addrs(struct tab_addrs *a) {
    a->addrs = NULL;
    a->len = 0;
}

void init_tab_requests(struct tab_requests *r) {
    for (int i = 0; i < r->len; i++) {
        r->requests[i].id = -1;
        r->requests[i].dest_addrs.addrs = NULL;
        r->requests[i].dest_addrs.len = 0;
        r->requests[i].index = 0;
        strcpy(r->requests[i].name, "\0");
    }
}

bool rm_request(struct tab_requests *r, int id) {
    int i;
    for (i = 0; i < r->len; i++) {
        if (r->requests[i].id == id) {
            r->requests[i].id = -1;
            free(r->requests[i].dest_addrs.addrs);
            r->requests[i].dest_addrs.len = 0;
            r->requests[i].index = 0;
            strcpy(r->requests[i].name, "\0");
        }
    }
    return i == r->len;
}

bool addr_cmp(struct sockaddr_in6 a1, struct sockaddr_in6 a2) {
    char ip1[IPLEN];
    char ip2[IPLEN];
    if (a1.sin6_family != a2.sin6_family) {
        return false;
    }
    if (a1.sin6_family == AF_INET6) {
        inet_ntop(AF_INET6, &a1.sin6_addr, ip1, sizeof(a1));
        inet_ntop(AF_INET6, &a2.sin6_addr, ip1, sizeof(a2));
    } else {
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&a1))->sin_addr, ip1, sizeof(a1));
        inet_ntop(AF_INET, &((struct sockaddr_in *)(&a2))->sin_addr, ip1, sizeof(a2));
    }
    return (ntohs(a1.sin6_port) == ntohs(a2.sin6_port)) && !strcmp(ip1, ip2);
}

void add_addr(struct sockaddr_in6 addr, struct tab_addrs *addrs) {
    if (addrs->addrs == NULL) {
        addrs->addrs = malloc(TABSIZE * sizeof(addr));
    }
    size_t size = addrs->len * sizeof(addr);
    if (size >= sizeof(addrs->addrs)) {
        addrs->addrs = realloc(addrs->addrs, size * INCREASE_COEF);
    }
    addrs->addrs[addrs->len] = addr;
    addrs->len++;
}

void rm_addr(struct sockaddr_in6 addr, struct tab_addrs *addrs) {
    int i;
    for (i = 0; i < addrs->len && !addr_cmp(addr, addrs->addrs[i]); i++)
        ;
    if (i != addrs->len) {}
}

void ignore(struct sockaddr_in6 addr, struct tab_addrs *ignored) {
    if (ignored->len < MAX_IGNORED) {
        add_addr(addr, ignored);
    } else {
        ignored->addrs[ignored->len % MAX_IGNORED] = addr;
        ignored->len++;
    }
}

bool is_ignored(struct sockaddr_in6 addr, struct tab_addrs ignored) {
    int i;
    for (i = 0; i < ignored.len && !addr_cmp(ignored.addrs[i], addr); i++)
        ;
    return i != ignored.len;
}

void send_req(int soc, struct request *request, struct tab_addrs ignored, struct tab_addrs *used, bool monitoring) {
    char req[REQLEN];

    struct timeval t;
    PCHK(gettimeofday(&t, NULL));

    int len_req;
    if ((len_req = snprintf(req, REQLEN, "%d|%ld,%ld|%s", request->id, t.tv_sec, t.tv_usec, request->name)) > REQLEN - 1)
        fprintf(stderr, "Request too long");

    if (monitoring)
        fprintf(stderr, "req: %s\n", req);

    bool send = false;
    for (size_t i = request->index; i < request->dest_addrs.len && send; i++) {
        if (!is_ignored(request->dest_addrs.addrs[i], ignored)) {  //&& minimal_use()) {
            PCHK(sendto(soc, req, len_req + 1, 0, (struct sockaddr *)&request->dest_addrs.addrs[i], (socklen_t)sizeof(struct sockaddr_in6)));
            request->index = i;
            add_use(request->dest_addrs.addrs[i], used);
            send = true;
        }
    }
}

struct res receive_res(int soc, struct tab_addrs ignored, struct tab_addrs *used, bool monitoring) {
    char res[REQLEN];

    struct sockaddr_in6 src_addr;
    socklen_t len_addr = sizeof(struct sockaddr_in6);
    ssize_t len_res;

    PCHK(len_res = recvfrom(soc, res, REQLEN, 0, (struct sockaddr *)&src_addr, &len_addr));
    rm_use(src_addr, used);
    struct res struc_res = parse_res(res);

    if (monitoring) {
        fprintf(stderr, "res: %s\n", res);
        fprintf(stderr, "in: %lds %ldms\n\n", struc_res.time.tv_sec, struc_res.time.tv_usec / 1000);
    }

    return struc_res;
}

void read_input(FILE *stream, int soc, int *id, struct tab_requests *tab_request, struct tab_addrs root_addr, struct tab_addrs ignored, struct tab_addrs *used, bool *goon, bool *monitoring) {
    char input[NAMELEN];
    if (fscanf(stream, "%s", input) == EOF) {
        if (ferror(stream)) {
            perror("fscanf(req_file,\"%s\", name)");
            exit(EXIT_FAILURE);
        }
    } else {
        if (*input != '!') {
            struct request req = {*id, input, root_addr, 0};
            *id += 1;
            send_req(soc, &req, ignored, used, monitoring);
            add_request(tab_request, req);
        } else {
            if (!strcmp(input, "!stop")) {
                goon = false;
            } else if (!strcmp(input, "!monitoring")) {
                *monitoring = !*monitoring;
                if (monitoring) {
                    fprintf(stderr, "monitoring:enabel");
                } else {
                    fprintf(stderr, "monitoring:disabel");
                }
            } else if (!strcmp(input, "!ignored")) {
                fprintf(stderr, "ignored server:\n");
                for (int i = 0; i < ignored.len; i++) {
                    fprint_addr(stderr, ignored.addrs[i]);
                }
                fprintf(stderr, "\n");
            }
        }
    }
}

void read_network(int soc, int *id, struct tab_requests *tab_request, struct tab_addrs root_addr, struct tab_addrs ignored, struct tab_addrs *used, bool *monitoring) {
    struct res res = receive_res(soc, ignored, used, monitoring);
    struct request *active_request = rech_req(tab_request, res.id);
    if (active_request == NULL) {
        return;
    }
    free(active_request->dest_addrs.addrs);
    active_request->dest_addrs = res.addrs;

    if (res.code > 0) {
        if (!strcmp(res.req_name, res.name)) {
            //print atbb addr
            rm_request(tab_request, res.id);
        } else {
            active_request->id = *id;
            active_request->index = 0;
            send_req(soc, active_request, ignored, used, monitoring);
            *id += 1;
        }
    } else {
        rm_request(tab_request, res.id);
        printf("\33[1;31mNot found\033[0m\n");
    }
}

int main(int argc, char const *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: <path of config file>");
        exit(EXIT_FAILURE);
    }

    FILE *req_file;

    int soc;

    struct tab_addrs root_addr, ignored, used;
    struct tab_requests requests;

    bool monitoring = false;
    bool goon = true;
    bool interactif = true;

    int id = 0;
    fd_set ensemble;
    struct timeval timeout_loop = {1, 0};

    init_tab_addrs(&root_addr);
    init_tab_addrs(&ignored);
    init_tab_addrs(&used);
    init_tab_requests(&requests);

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
            read_input(stdin, soc, &id, &requests, root_addr, ignored, &used, &goon, &monitoring);
        } else if (FD_ISSET(soc, &ensemble)) {
            read_network(soc, &id, &requests, root_addr, ignored, &used, &monitoring);
        } else {
        }
    }

    while (goon && !interactif) {
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);
        PCHK(select(soc + 1, &ensemble, NULL, NULL, &timeout_loop));

        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            read_input(stdin, soc, &id, &requests, root_addr, ignored, &used, &goon, &monitoring);
        } else if (FD_ISSET(soc, &ensemble)) {
            read_network(soc, &id, &requests, root_addr, ignored, &used, &monitoring);
        } else {
        }
    }

    if (!interactif) {
        fclose(req_file);
    }

    free(root_addr.addrs);
    free(ignored.addrs);
    free(used.addrs);
    exit(EXIT_SUCCESS);
}
