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
        printf("Usage: chmod <path> <mode>\n");
        exit(1);
    }
    
    char *path = argv[1];
    uint32_t mode = atoi((const uint8_t*)argv[2]);
    
    if (chmod(path, mode) == 0) {
        printf("chmod: changed '%s' to %d\n", path, mode);
        exit(0);
    } else {
        printf("chmod: cannot change '%s'\n", path);
        exit(1);
    }
}
