#include "constants.h"
#include "sched.h"
#include "arch/trap.h"
#include "arch/stdio.h"
#include "arch/mem.h"
#include "arch/cpus.h"
#include "arch/trap_handling.h"
#include "arch_inc/trap_constants.h"
#include "stdio.h"
#include "console/debug.h"
#include "console_files.h"
#include "proc_fs.h"

#include "arch/arch_init.h"

// When nothing more to be executed on shell!
void sched_finish(struct Proc * last_proc){

    
    if(last_proc){
        // Just printf
        // printf("++++++++ Current is blocked and no other ready proc.. waiting..\n"); 
        switch_to_idle_proc();        
    }
    printf("[INFO] tot idle ticks: %u tot ticks = %u\n",get_idle_ticks(), get_real_ticks());
    enable_interrupts();
    enable_timer_interrupts();

    switch_to_idle_proc();
}

// void *mboot, unsigned int magic_number
// UNUSED_ARGUMENT(mboot);
//     UNUSED_ARGUMENT(magic_number);
// return 0xDEADBEEF;


// Riscv would jump straight to this, because entry point does not jump to kmain
// on secondary cpus
void secondary_cpu_main(){
    // Secondary cpus also need to init trap and clock interrupts.. this init trap
    // Does not leave clock interrupts enabled, so we can control when to enable it on secondary cpus
	#ifdef IS_RISC 
    init_trap(); 
   	#endif
   
    init_sched_secondary_cpu();
    for (;;)
    {
        // __asm__ __volatile__("wfi");
    }
}

volatile static int started = 0;

void kmain()
{

    disable_debug_print();
    #ifdef IS_RISC
    #else
    mem_init(); // first of all set up paging
    #endif
    init_arch();
    disable_debug_print();
    reset_std_files();
    clear();
    move_cursor(0);

    debug_printf("HOLIS\n");

    init_trap();

    init_disk(); 



    #ifdef IS_RISC
    mem_init(); // first of all set up paging
    #endif
    
    // main_tests();

    init_syscalls_ipc();
    init_syscalls_proc();
    init_proc_mem_management();
    init_system_info();
    
    #ifdef IS_RISC
    #else
    kbd_hw_enable();
    kbd_init();
    #endif
    
    disable_debug_print();
    debug_printf("\n\nHello World!\n");
    

    printf("INITING IDLE PROC In case all procs are blocked!\n");        
    init_idle_proc();
    
    printf("INITING USER APP HEADERS!\n");        
    init_proc_headers();

    init_sched_main_cpu();

    for (;;)
    {
        // __asm__ __volatile__("wfi");
    }
}


