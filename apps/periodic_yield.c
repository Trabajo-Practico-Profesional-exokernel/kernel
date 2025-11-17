#include "lib.h"
#define SLEEP_TIME 300000000

void main() {
    int count = 0;;
    while (1){
        count+=1;
        printf("BEFORE YIELD N %d!\n",count);
        sys_yield();
    }
}