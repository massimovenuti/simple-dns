#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "macro.h"
#include "parser.h"

#define REQLEN 512


struct request {
    int id;
    char name[NAMELEN];
};

typedef struct s_node {
    char name[NAMELEN];
    struct addr_with_flag tab_addr[MAX_ADDR]; // à allouer dynamiquement...
    int nb_addrs;
    int index;
    struct s_node *sons[MAX_SONS];
    int nb_sons;
} node, *tree;

tree new_tree(char *name, struct addr_with_flag *addrs);

tree adj_son(tree t, char *name, struct addr_with_flag *addrs);

void destroy(tree t);

tree rech_inter(char *name, tree t, int ind);

void ignore(struct addr_with_flag addr, struct ignored_servers *servers);

void print_tree(tree t);

tree rech(char *name, tree t);

char *resolve(int soc, struct request *request, char *dst, tree tree_addr, struct ignored_servers *ignored_serv, bool monitoring, bool free_tab);