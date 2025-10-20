#include "inc/syscalls.h"
#include "inc/common.h"
#include "lib.h"

#define SLEEP_TIME 300000000
//__attribute__((section(".user_func")))
void main() {
    printf("hello Shell!\n");
    while (1){
        // putchar('h');
        // putchar('e');
        // putchar('l');
        // putchar('l');
        // putchar('o');
        // putchar(' ');
        // putchar('S');
        // putchar('h');
        // putchar('e');
        // putchar('l');
        // putchar('l');
        // putchar('\n');
        sleep(SLEEP_TIME);
        printf("Iteration!\n");
    }
}