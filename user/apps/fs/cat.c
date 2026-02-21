#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "types.h"

void main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: cat <filename>\n");
        exit(1);
    }
    
    char *filename = argv[1];
    int fd = open(filename, 0);  // 0 = O_RDONLY
    
    if (fd < 0) {
        printf("cat: cannot open '%s'\n", filename);
        exit(1);
    }
    
    char buf[512];
    int bytes;
    
    while ((bytes = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = '\0';
        printf("%s", buf);
    }
    
    if (bytes < 0) {
        printf("cat: read error\n");
    }
    
    close(fd);
    exit(0);
}
