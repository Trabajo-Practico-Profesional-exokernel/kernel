#include "inc/common.h"
#include "sched.h"

#include "arch/trap.h"
#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "arch/mem.h"
#include "arch/trap_handling.h"




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

    #ifdef IS_RISC
    //Doing it after init_trap just to be able to see a trap/panic if something fails!
    // Mem init for riscv == setup pagetable for kernel.
    mem_init();
    // Why not ... maybe not full needed at first but works.
    switch_to_kernel_tables();

    init_syscalls_fs();
    init_syscalls_ipc();
    init_syscalls_proc();
    
    #endif

    printf("\n\nHello World!\n");
    
    init_sched();

    for (;;)
    {
        // __asm__ __volatile__("wfi");
    }
}
