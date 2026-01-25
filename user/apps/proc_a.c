#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"

#define SLEEP_TIME 300000000

void main() {
    int fd[2];
    printf("1. Calling pipe...\n");
    
    int result = pipe(fd);
    if (result < 0) {
        printf("Pipe failed\n");
        return;
    }
    printf("Pipe created. Read FD: [%d], Write FD: [%d]\n", fd[0], fd[1]);

    printf("2. Calling dup on Write FD [%d]...\n", fd[1]);
    
    int fd_dup = dup(fd[1]);
    
    printf("Result dup: [%d]\n", fd_dup);

    if (fd_dup < 0) {
        printf("Dup failed!\n");
        return;
    }

    if (fd_dup == fd[1]) {
        printf("Error: Dup returned the same FD number (it should be different)\n");
        return;
    }

    printf("3. Writing in pipe using DUPLICATED FD [%d]...\n", fd_dup);
    
    char msg[] = "hola via dup";
    int result_writing = write(fd_dup, msg, strlen(msg));

    if (result_writing < 0){
        printf("Pipe fail in write, error: [%d]\n", result_writing);
        return;
    }
    printf("Bytes written via dup: %d\n", result_writing);

    printf("4. Reading from pipe using ORIGINAL Read FD [%d]...\n", fd[0]);

    char buffer[30];
    memset(buffer, 0, 30); 

    int resul_reading = read(fd[0], buffer, 30);

    if (resul_reading < 0){
        printf("Pipe fail in read, error: [%d]\n", resul_reading);
        return;
    }

    buffer[resul_reading] = '\0'; 

    printf("Result reading: [%s]\n", buffer);

    printf("5. Closing all FDs...\n");
    close(fd[0]);
    close(fd[1]);
    close(fd_dup); 
    
    printf("Test finished.\n");
}