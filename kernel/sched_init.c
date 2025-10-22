#include "sched.h"
#include "proc.h"
#include "inc/common.h"

#include "arch/logging.h"
#include "arch/mem_layout.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.

#ifdef IS_RISC
void main_app_a();

// This is created on the shell.bin.o by the llvm copy thing or so. 
// _binary_ prefix is always there apps_build_shell_bin is the path to the binary basically
// Essentially is where and how big is the niary of the shell app.
extern char _binary_apps_build_shell_bin_start[], _binary_apps_build_shell_bin_size[];

// This is defined on link.ld of the kernel... to hardcode a simple user space page
extern char __user_space_start[], __user_space_end[];

void init_sched(void) {
    //proc_a = create_process((uint32_t) proc_a_entry);
    struct Proc * proc_a = get_first_free_proc();
    struct Proc * proc_b = get_first_free_proc();
    set_proc_a(proc_a);
    set_proc_b(proc_b);

    create_process_user(proc_a, (uint32_t) main_app_a
    	, (paddr_t) __user_space_start, (paddr_t) __user_space_end);
    
    load_create_process_user(proc_b,
    	_binary_apps_build_shell_bin_start, (size_t) _binary_apps_build_shell_bin_size);
    
    // create_process_user(proc_b, (uint32_t) main_app_b
    // 	, (paddr_t) __user_space_start, (paddr_t) __user_space_end);

    printf("AT CREATE PROCESS A expected pc= %x, ", (uint32_t) main_app_a);
    printProc(proc_a);
    
    printf("AT CREATE PROCESS SHELL expected pc= %x, ", (uint32_t)VADDR_USER_BASE);
    printProc(proc_b);
    // Start proc_a!
    switch_proc(proc_b);
    PANIC("unreachable here!");
}


#else // IS X86
#include "arch/switch.h"

#define SLEEP_TIME 300000000


void proc_a_entry(void) {
    printf("starting process A\n");
    //syscall(SYS_KALLOC, 1, 0, 0); //For when its on user space.
    //printf("called kalloc on A\n");
    while (1) {
        printf("A after sleep\n");
        sleep(SLEEP_TIME);
        //printProc(proc_a);
    }
}

void proc_b_entry(void) {
    printf("starting process B\n");
    while (1) {
        sleep(SLEEP_TIME);
        printf("B after sleep proc_b: \n");
    }
}

void init_sched(void) {
    struct Proc * proc_a = get_first_free_proc();
    struct Proc * proc_b = get_first_free_proc();
    set_proc_a(proc_a);
    set_proc_b(proc_b);
    
    create_process(proc_a, (uint32_t) proc_a_entry);
    create_process(proc_b,(uint32_t) proc_b_entry);
    printf("AT CREATE PROCESS A expected pc= %u, \n", (uint32_t) proc_a_entry);
    printProc(proc_a);
    
    printf("AT CREATE PROCESS B expected pc= %u, \n", (uint32_t) proc_b_entry);
    printProc(proc_b);

    printf("START!\n");
    // DEBERIA LLAMAR SOLO A SWITCH PROC.. no setear a mano curr 
    //switch_proc(proc_a);

    set_curr(proc_a);
    //switch_page_table(curr->page_table, &curr->stack[sizeof(curr->stack)]);
    proc_a_entry();

    PANIC("unreachable here!");
}



#endif