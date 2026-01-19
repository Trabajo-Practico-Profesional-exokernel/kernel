#include "inc/common.h"
#include "sched.h"
#include "../test/testing.h"
#include "arch/trap.h"
#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "arch/mem.h"
#include "arch/trap_handling.h"
#include "fd.h"

// void *mboot, unsigned int magic_number
// UNUSED_ARGUMENT(mboot);
//     UNUSED_ARGUMENT(magic_number);
// return 0xDEADBEEF;
void kmain()
{
    disable_debug_print();
    init_arch();
    clear();
    move_cursor(0);
    debug_printf("HOLIS\n");

    init_trap();

    init_disk(); 
    //Doing it after init_trap just to be able to see a trap/panic if something fails!
    // Mem init for riscv == setup pagetable for kernel.
    mem_init();
    #ifdef IS_RISC
    // Why not ... maybe not full needed at first but works.
    switch_to_kernel_tables();
    #endif
    
    main_tests();

    init_files();
    init_syscalls_files();
    init_syscalls_ipc();
    init_syscalls_proc();
    init_user_pages_alloc();

    #ifdef IS_RISC
    #else
    kbd_hw_enable();
    kbd_init();
    #endif
    disable_debug_print();
    debug_printf("\n\nHello World!\n");
    
    init_sched();

    for (;;)
    {
        // __asm__ __volatile__("wfi");
    }
}
