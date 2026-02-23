#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "parsers/strutil.h"

void main(int argc, char **argv) {
    int newline = 1;
    int start = 1;

    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        newline = 0;
        start = 2;
    }

    for (int i = start; i < argc; i++) {
        printf("%s", argv[i]);
        if (i < argc - 1)
            printf(" ");
    }

    if (newline)
        printf("\n");

    exit(0);
}