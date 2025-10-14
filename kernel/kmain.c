#include "stdio.h"

#include "common.h"

// void *mboot, unsigned int magic_number
// UNUSED_ARGUMENT(mboot);
//     UNUSED_ARGUMENT(magic_number);
    // return 0xDEADBEEF;
void kmain()
{
    clear();
    move_cursor(0);
    printf("HOLIS");
    for (;;) {
        // __asm__ __volatile__("wfi");
    }
}
