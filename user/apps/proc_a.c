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
}