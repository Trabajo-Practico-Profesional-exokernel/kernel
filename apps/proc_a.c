#include "lib.h"
#define SLEEP_TIME 300000000

void main() {
    int count = 0;
    while (count < 26){
        printf("PROC A %d!\n", count);
        sleep(SLEEP_TIME);
        count+=1;
    }
}