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
        printf("Usage: stat <path>\n");
        exit(1);
    }
    
    if (stat(argv[1]) == 0) {
        exit(0);
    } else {
        printf("stat: cannot stat '%s'\n", argv[1]);
        exit(1);
    }
}
