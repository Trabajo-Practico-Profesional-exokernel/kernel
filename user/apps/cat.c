#include "syscalls.h"
#include "inc/filesystem.h"
#include "constants.h"
#include "stdio.h"
#include "arch/console.h"

#define BUFFER_SIZE 512

void cat_fd(int fd) {
    char buf[BUFFER_SIZE];
    int n;
    
    while ((n = read(fd, buf, BUFFER_SIZE)) > 0) {
        if (write(STDOUT, buf, n) != n) {
            printf("cat: write error\n");
            return;
        }
    }
    if (n < 0) {
        printf("cat: read error\n");
    }
}

int main(int argc, char *argv[]) {
    int fd;

    if (argc <= 1) {
        cat_fd(STDIN);
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        fd = open(argv[i], 1);
        if (fd < 0) {
            printf("cat: cannot open %s\n", argv[i]);
            continue;
        }
        cat_fd(fd);
        close(fd);
    }
    
    return 0;
}