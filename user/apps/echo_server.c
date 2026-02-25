
#include "syscalls.h"
#include "string.h"
#include "constants.h"
#include "arch/console.h"
#include "stdio.h"


int main(int argc, char *argv[]) {
    char buff[512];
    int file_fd = -1;
    
    // Open output file if provided as argument
    if (argc > 1) {
        file_fd = open(argv[1], 2); // 1 = O_WRONLY
        if (file_fd < 0) {
            printf("Error: Cannot open file for output: %s\n", argv[1]);
            exit(-1);
        }
    }
    
    // Read from stdin and write to both stdout and file
    int bytes;
    while ((bytes = read(STDIN, buff, sizeof(buff))) > 0) {
        // Write to stdout
        write(STDOUT, buff, bytes);
        // Write to file if opened
        if (file_fd >= 0) {
            write(file_fd, buff, bytes);
        }
    }
    
    // Close file if opened
    if (file_fd >= 0) {
        close(file_fd);
    }
    
    return 0;
}
