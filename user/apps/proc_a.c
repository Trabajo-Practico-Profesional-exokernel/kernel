#include "lib.h"
#include "syscalls.h"
#include "std/string.h"

#define SLEEP_TIME 300000000

void main() {
    int count = 0;
    int fd[2];
    printf("calling pipe\n");
    int result = pipe(fd);
    printf("result pipe: [%d]\n", result);
    printf("writing in pipe\n");
    char msg[]= "hola como andas";
    int result_writing = write(fd[1], msg, strlen(msg));

    if (result_writing < 0){
        printf("pipe fail in write, error: [%d]\n", result_writing);
        return;
    }

    char buffer[30];
    int resul_reading = read(fd[0], buffer, 30);

    if (resul_reading < 0){
        printf("pipe fail in read, error: [%d]\n", resul_reading);
        return;
    }

    printf("result reading: [%s]\n", buffer);
    
}