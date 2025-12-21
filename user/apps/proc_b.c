#include "lib.h"
#define SLEEP_TIME 300000000

void main() {
    int count = 0;

    while (1){
        printf("PROC B %d!\n", count);
        sleep(SLEEP_TIME);
        count+=1;
    }
}