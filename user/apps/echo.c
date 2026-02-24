
#include "syscalls.h"
#include "string.h"
#include "constants.h"
#include "arch/console.h"
#include "stdio.h"


int main(int argc, char *argv[]) {
    int newline = 1;
    int start = 1;

    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        newline = 0;
        start = 2;
    }

    for (int i = start; i < argc; i++) {
        write(STDOUT, argv[i], strlen((const uint8_t *)argv[i]));
        if (i < argc - 1) {
            write(STDOUT, " ", 1);
        }
    }
    if (newline)
        write(STDOUT, "\n", 1);
    return 0;
}
