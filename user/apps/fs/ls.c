#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "types.h"

void main(int argc, char **argv) {
    char *path = NULL;
    
    // Default to current directory if no argument
    if (argc < 2) {
        path = ".";
    } else {
        path = argv[1];
    }
    
    if (ls(path) != 0) {
        printf("ls: cannot access '%s'\n", path);
        exit(1);
    }
    printf("ls: finished '%s'\n", path);
    
    exit(0);
}
