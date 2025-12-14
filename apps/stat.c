#include "lib.h"

#define SLEEP_TIME 300000000
#define MSG_SIZE_MAX 64

void main() {
    char * name = "some_file.txt";

    int fd = sys_rm(name);

    printf("STAT GOT RES %d", fd);
}