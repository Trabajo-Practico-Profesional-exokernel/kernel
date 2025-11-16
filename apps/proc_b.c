#include "lib.h"
#define SLEEP_TIME 300000000

void main() {

    while (1){
        printf("PROC B!\n");
        sleep(SLEEP_TIME);
    }
}