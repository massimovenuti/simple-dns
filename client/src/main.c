#include "main.h"

void ignore(struct sockaddr_in6 addr, struct ignored_servers *servers) {
    struct server tmp = addr_to_string(addr);
    MCHK(strcpy(servers->servers[servers->nb_servers].ip, tmp.ip));
    MCHK(strcpy(servers->servers[servers->nb_servers].port, tmp.port));
    servers->nb_servers = (servers->nb_servers + 1) % MAX_IGNORED;
}

void send_req(int soc, struct request *request, struct ignored_servers servers, struct ignored_servers *use, bool monitoring) {
    char req[REQLEN];

    struct timeval t;
    PCHK(gettimeofday(&t, NULL));

    int len_req;
    if ((len_req = snprintf(req, REQLEN, "%d|%ld,%ld|%s", request->id, t.tv_sec, t.tv_usec, request->name)) > REQLEN - 1)
        fprintf(stderr, "Request too long");

    if (monitoring)
        fprintf(stderr, "req: %s\n", req);

    bool send = false;
    for (size_t i = request->index_addr; i < request->tab_dst.len && send; i++) {
        if (!is_ignored() && minimal_use()) {
            PCHK(sendto(soc, req, len_req + 1, 0, (struct sockaddr *)&request->tab_dst.addr[i], (socklen_t)sizeof(struct sockaddr_in6)));
            request->index_addr = i;
            add_use(request->tab_dst.addr[i], use);
            send = true;
        }
    }
}

struct res receive_res(int soc, struct ignored_servers servers, struct ignored_servers *use, bool monitoring) {
    char res[REQLEN];

    struct sockaddr_in6 src_addr;
    socklen_t len_addr = sizeof(struct sockaddr_in6);
    ssize_t len_res;

    PCHK(len_res = recvfrom(soc, res, REQLEN, 0, (struct sockaddr *)&src_addr, &len_addr));
    rm_use(src_addr, use);
    struct res struc_res = parse_res(res, len_res, servers);

    if (monitoring) {
        fprintf(stderr, "res: %s\n", res);
        fprintf(stderr, "in: %lds %ldms\n\n", struc_res.time.tv_sec, struc_res.time.tv_usec / 1000);
    }

    return struc_res;
}

void read_input(FILE *stream, int soc, int *id, struct tab_requests *tab_request, struct tab_addr root_addr, struct ignored_servers servers, struct ignored_servers *use, bool *goon, bool *monitoring) {
    char input[NAMELEN];
    if (fscanf(stream, "%s", input) == EOF) {
        if (ferror(stream)) {
            perror("fscanf(req_file,\"%s\", name)");
            exit(EXIT_FAILURE);
        } else {
            goon = false;
        }
    } else {
        if (*input != '!') {
            struct request req = {*id, input, root_addr, 0};
            *id += 1;
            send_req(soc, &req, servers, use, monitoring);
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
                for (int i = 0; i < servers.nb_servers; i++) {
                    fprintf(stderr, "%s:%s\n", servers.servers[0].ip, servers.servers[0].port);
                }
                fprintf(stderr, "\n");
            }
        }
    }
}

void read_network(int soc, int *id, struct tab_requests *tab_request, struct tab_addr root_addr, struct ignored_servers servers, struct ignored_servers *use, bool *monitoring) {
    struct res res = receive_res(soc, servers, use, monitoring);
    struct request *active_request = rech_req(tab_request, res.id);
    if (active_request == NULL) {
        return;
    }
    free(active_request->tab_dst.addr);
    active_request->tab_dst = res.addrs;

    if (res.code > 0) {
        if (!strcmp(res.req_name, res.name)) {
            //print atbb addr
            rm_request(tab_request, res.id);
        } else {
            active_request->id = id;
            active_request->index_addr = 0;
            send_req(soc, active_request, servers, use, monitoring);
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

    struct tab_addr root_addr;
    struct ignored_servers ignored_serv;
    struct tab_requests requests;
    ignored_serv.nb_servers = 0;

    root_addr = parse_conf(argv[1]);

    int soc;
    PCHK(soc = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP));

    bool interactif = true;
    FILE *req_file;
    if (argc == 3) {
        interactif = false;
        MCHK(req_file = fopen(argv[2], "r"));
    }

    int id = 0;
    fd_set ensemble;
    struct timeval timeout_loop = {1, 0};
    bool monitoring = false;
    bool goon = true;

    while (goon && interactif) {
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);
        PCHK(select(soc + 1, &ensemble, NULL, NULL, &timeout_loop));

        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            read_input(stdin, soc, &id, &requests, root_addr, ignored_serv, &use, &goon, &monitoring);
        } else if (FD_ISSET(soc, &ensemble)) {
            read_network(soc, &id, &requests, root_addr, ignored_serv, &use, monitoring);
        } else {
        }
    }

    while (goon && !interactif) {
        FD_ZERO(&ensemble);
        FD_SET(STDIN_FILENO, &ensemble);
        FD_SET(soc, &ensemble);
        PCHK(select(soc + 1, &ensemble, NULL, NULL, &timeout_loop));

        if (FD_ISSET(STDIN_FILENO, &ensemble)) {
            read_input(stdin, soc, &id, &requests, root_addr, ignored_serv, &use, &goon, &monitoring);
        } else if (FD_ISSET(soc, &ensemble)) {
            read_network(soc, &id, &requests, root_addr, ignored_serv, &use, monitoring);
        } else {
        }
    }

    if (!interactif) {
        fclose(req_file);
    }

    free(root_addr.addr);
    exit(EXIT_SUCCESS);
}
