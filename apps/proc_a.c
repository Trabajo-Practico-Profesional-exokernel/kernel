#include "lib.h"
#include "syscalls.h"
#define SLEEP_TIME 300000000

void main() {
    int count = 0;
    while (count < 26){
        printf("PROC A %d! My PID is [%u], UPTIME: [%u]\n", count, getpid(), uptime());
        sleep(SLEEP_TIME);
        count+=1;
    }
}