#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "types.h"

void main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: unlink <path>\n");
        exit(1);
    }
    
    if (unlink(argv[1]) == 0) {
        printf("unlink: removed '%s'\n", argv[1]);
        exit(0);
    } else {
        printf("unlink: cannot remove '%s'\n", argv[1]);
        exit(1);
    }
}
