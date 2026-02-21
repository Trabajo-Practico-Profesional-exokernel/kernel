#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "types.h"

void main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: mkdir <path>\n");
        exit(1);
    }
    
    if (mkdir(argv[1]) == 0) {
        printf("mkdir: created '%s'\n", argv[1]);
        exit(0);
    } else {
        printf("mkdir: cannot create '%s'\n", argv[1]);
        exit(1);
    }
}
