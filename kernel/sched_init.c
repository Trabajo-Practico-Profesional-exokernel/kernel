#include "sched.h"
#include "proc.h"
#include "inc/common.h"

#include "arch/logging.h"
#include "arch/mem_layout.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.

#ifdef IS_RISC
#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  

// meta/gen/apps_meta.c defines this...
extern struct AppBinaryInfo _binary_apps[];


// void main_app_a();



// This is defined on link.ld of the kernel... to hardcode a simple user space page
extern char __user_space_start[], __user_space_end[];

void init_sched(void) {

    // Main user process
    struct Proc * proc_shell = get_first_free_proc();
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_PROC_A]);
    load_create_process_user(proc_shell, &_binary_apps[APP_IND_SHELL]);
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_TESTS_SHELL]);

    // Extra initial processes....
    load_create_process_user(get_first_free_proc(), 
                        &_binary_apps[APP_IND_PROC_B]);

    
    printf("AT CREATE PROCESS SHELL expected pc= %x, ", (uint32_t)VADDR_USER_BASE);
    printProc(proc_shell);

    
    // Start proc_shell!
    switch_proc(proc_shell);
    
    PANIC("unreachable here!");
}


#else // IS X86
#include "arch/switch.h"

#define SLEEP_TIME 150000000


void proc_a_entry(void) {
    printf("starting process A\n");
    //syscall(SYS_KALLOC, 1, 0, 0); //For when its on user space.
    //printf("called kalloc on A\n");
    while (1) {
        // printf("A after sleep\n");
        sleep(SLEEP_TIME);
        //printProc(proc_a);
    }
}

void proc_b_entry(void) {
    printf("starting process B\n");
    while (1) {
        // printf("B after sleep proc_b: \n");
        sleep(SLEEP_TIME);
    }
}

// #include "arch/mem.h" //defines perms like PAGE_R and so on.
//#include "arch_inc/trapframe.h"
//extern char __free_ram[], __free_ram_end[];

void init_sched(void) {
    // paddr_t new_stk_base = alloc_pages(2);
    // paddr_t new_stk_top = new_stk_base+2*PAGE_SIZE;

    // printf("SOME LOG? %x to %x\n" ,new_stk_base, new_stk_top);
    // SWITCH_TO_STACK(new_stk_base, new_stk_top);

    struct Proc * proc_a = get_first_free_proc();
    struct Proc * proc_b = get_first_free_proc();
    set_proc_a(proc_a);
    set_proc_b(proc_b);
    
    create_process(proc_a, (uint32_t) proc_a_entry);
    create_process(proc_b,(uint32_t) proc_b_entry);
    printf("[INIT SCHED] AT CREATE PROCESS A expected pc= %u, \n", (uint32_t) proc_a_entry);
    printf("[INIT SCHED] ");
    printProc(proc_a);
    
    printf("[INIT SCHED] AT CREATE PROCESS B expected pc= %u, \n", (uint32_t) proc_b_entry);
    printf("[INIT SCHED] ");
    printProc(proc_b);

    printf("START!\n");
    // DEBERIA LLAMAR SOLO A SWITCH PROC.. no setear a mano curr 
    switch_proc(proc_a);

    //set_curr(proc_a);
    //switch_page_table(curr->page_table, &curr->stack[sizeof(curr->stack)]);
    //proc_a_entry();

    PANIC("unreachable here!");
}



#endif