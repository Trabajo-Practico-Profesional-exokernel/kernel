#include "inc/common.h"
#include "sched.h"

#include "arch/trap.h"
#include "arch/stdio.h"
#include "arch/arch_init.h"




// void *mboot, unsigned int magic_number
// UNUSED_ARGUMENT(mboot);
//     UNUSED_ARGUMENT(magic_number);
// return 0xDEADBEEF;
void kmain()
{

    init_arch();

    clear();
    move_cursor(0);
    printf("HOLIS\n");
    init_trap();

    printf("\n\nHello World!\n");
    
    init_sched();

    for (;;)
    {
        // __asm__ __volatile__("wfi");
    }
}
