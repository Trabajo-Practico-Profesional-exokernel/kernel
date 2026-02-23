#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "types.h"
#include "optional_setup.h"

void main(int argc, char **argv) {
    setup_from_env();
    if (argc < 2) {
        printf("Usage: rmdir <path>\n");
        exit(1);
    }
    
    if (rmdir(argv[1]) == 0) {
        printf("rmdir: removed '%s'\n", argv[1]);
        exit(0);
    } else {
        printf("rmdir: cannot remove '%s'\n", argv[1]);
        exit(1);
    }
}
