#include "main.h"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: <path of config file>");
    }

    struct type_addr *tab_addr;
    tab_addr = parse(argv[1]);
    free(tab_addr);

    exit(EXIT_SUCCESS);
}
