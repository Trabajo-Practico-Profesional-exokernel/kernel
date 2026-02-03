#include "constants.h"
#include "sched.h"
#include "arch/trap.h"
#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "arch/mem.h"
#include "arch/cpus.h"
#include "arch/trap_handling.h"
// #include "fd.h"
#include "stdio.h"
#include "console/debug.h"
#include "arch_inc/trap_constants.h"

#include "interactive_test_interpreter.h"
#include "interactive_test_commands.h"

//
// void *mboot, unsigned int magic_number
// UNUSED_ARGUMENT(mboot);
//     UNUSED_ARGUMENT(magic_number);
// return 0xDEADBEEF;

// When nothing more to be executed on shell!
void sched_finish(bool current_is_blocked){
    resume_interactive_shell();
}


// Riscv would jump straight to this, because entry point does not jump to kmain
// on secondary cpus
void secondary_cpu_main(){
    add_start_cpu();
    for (;;)
    {
        // __asm__ __volatile__("wfi");
    }       
}

void kmain()
{

    init_cpus(); // First init cpus, here we set the cpuid

    #ifdef IS_RISC
    #else
    // Halt other cpus if not main one to init kernel.
    // Riscv opensbi already does this, so its in theory for x86. Or just in case.
    // IN RISCV opensbi does not guarantee that cpuid == 0 is the boot one.
    if(cpuid() != 0){ 
        wait_start_cpus();
                
        secondary_cpu_main();

        PANIC("Should not reach here secondary cpu!");
    }
    #endif

    disable_debug_print();
    init_arch();
    clear();
    move_cursor(0);

    printf("==>TEST INIT TRAP!\n");
    init_trap();

    printf("==>TEST INIT DISK!\n");
    init_disk(); 
    //Doing it after init_trap just to be able to see a trap/panic if something fails!
    // Mem init for riscv == setup pagetable for kernel.
    printf("==>TEST INIT MEM!\n");
    mem_init();

    #ifdef IS_RISC
    // Why not ... maybe not full needed at first but works.
    switch_to_kernel_tables();
    #endif
    

    printf("==>TEST INIT SYSCALLS!\n");
    // init_files();
    init_syscalls_ipc();
    init_syscalls_proc();
    init_user_pages_alloc();

    #ifdef IS_RISC
    #else
    kbd_hw_enable();
    kbd_init();
    #endif
    disable_debug_print();
    // No lock needed for this set since is just 1 writer and once!
    // In riscv is needed, since we are using opensbi, opensbi halts the cpus until notified.
    // Like we would do with started == 0.
    disable_timer_interrupts();
    

    init_interactive_tests();

    for (;;)
    {
        // __asm__ __volatile__("wfi");
    }
}
