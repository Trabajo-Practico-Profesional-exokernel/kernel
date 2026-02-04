#include "constants.h"
#include "sched.h"
#include "arch/trap.h"
#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "arch/mem.h"
#include "arch/cpus.h"
#include "arch/trap_handling.h"
#include "arch_inc/trap_constants.h"
#include "stdio.h"
#include "console/debug.h"
#include "console_files.h"


// When nothing more to be executed on shell!
void sched_finish(bool curr_is_blocked){

    if(curr_is_blocked){
        // Just printf
        // printf("++++++++ Current is blocked and no other ready proc.. waiting..\n"); 
        switch_to_idle_proc();        
    }
    printf("[INFO] tot idle ticks: %u tot ticks = %u\n",get_idle_ticks(), get_real_ticks());

    PANIC("+++++++++++++++++++++ Nothing to run at sched yield!?\n");    
}

// void *mboot, unsigned int magic_number
// UNUSED_ARGUMENT(mboot);
//     UNUSED_ARGUMENT(magic_number);
// return 0xDEADBEEF;


// Riscv would jump straight to this, because entry point does not jump to kmain
// on secondary cpus
void secondary_cpu_main(){
    printf("Should start cpu %d\n", cpuid());
    init_sched();
    
    for (;;)
    {
        // __asm__ __volatile__("wfi");
    }
}

volatile static int started = 0;

void kmain()
{

    init_cpus(); // First init cpus, here we set the cpuid
    


    #ifdef IS_RISC
    #else
    // Halt other cpus if not main one to init kernel.
    // Riscv opensbi already does this, so its in theory for x86. Or just in case.
    // IN RISCV opensbi does not guarantee that cpuid == 0 is the boot one.
    if(cpuid() != 0){ 
        printf("Does dis work? %d \n", cpuid());
        while(started == 0)
              ;

        secondary_cpu_main();

        PANIC("Should not reach here secondary cpu!");
    }
    #endif

    disable_debug_print();
    init_arch();
    reset_std_files();
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
    
    // main_tests();

    init_syscalls_ipc();
    init_syscalls_proc();
    init_proc_mem_management();

    #ifdef IS_RISC
    #else
    kbd_hw_enable();
    kbd_init();
    #endif
    disable_debug_print();
    debug_printf("\n\nHello World!\n");
    
    // No lock needed for this set since is just 1 writer and once!
    started = 1;

    // In riscv is needed, since we are using opensbi, opensbi halts the cpus until notified.
    // Like we would do with started == 0.
    notify_inited();
    printf("---> x86 start scged \n");
    init_sched();

    for (;;)
    {
        // __asm__ __volatile__("wfi");
    }
}
