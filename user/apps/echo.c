#include "syscalls.h"
#include "string.h"
#include "constants.h"
#include "arch/console.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        write(STDOUT, argv[i], strlen((const uint8_t *)argv[i]));
        if (i < argc - 1) {
            write(STDOUT, " ", 1);
        }
    }
    write(STDOUT, "\n", 1);
    return 0;
}
