#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "types.h"

void main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: touch <filename>\n");
        exit(1);
    }
    
    if (mknod(argv[1], 0, 0) == 0) {
        printf("touch: created '%s'\n", argv[1]);
        exit(0);
    } else {
        printf("touch: cannot create '%s'\n", argv[1]);
        exit(1);
    }
}
