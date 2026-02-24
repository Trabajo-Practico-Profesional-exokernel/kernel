#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "types.h"
#include "optional_setup.h"

void main(int argc, char **argv) {
    setup_from_env();
    
    if (argc < 3) {
        printf("Usage: link <old_path> <new_path>\n");
        exit(1);
    }
    
    if (link(argv[1], argv[2]) == 0) {
        printf("link: created link '%s' -> '%s'\n", argv[2], argv[1]);
        exit(0);
    } else {
        printf("link: cannot create link\n");
        exit(1);
    }
}
