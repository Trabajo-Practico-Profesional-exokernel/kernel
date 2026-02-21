#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "types.h"

void main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: chown <path> <uid> <gid>\n");
        exit(1);
    }
    
    char *path = argv[1];
    uint32_t uid = atoi((const uint8_t*)argv[2]);
    uint32_t gid = atoi((const uint8_t*)argv[3]);
    
    if (chown(path, uid, gid) == 0) {
        printf("chown: changed '%s' to %d:%d\n", path, uid, gid);
        exit(0);
    } else {
        printf("chown: cannot change '%s'\n", path);
        exit(1);
    }
}
