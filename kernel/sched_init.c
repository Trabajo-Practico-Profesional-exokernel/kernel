#include "sched.h"
#include "proc.h"
#include "inc/common.h"

#include "arch/logging.h"
#include "arch/mem_layout.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  

char *DEF_ARGV[] = { "sh_prog","parameter1", 0 };

// This is defined on link.ld of the kernel... to hardcode a simple user space page
#ifdef IS_RISC
// meta/gen/apps_meta.c defines this...
extern struct AppBinaryInfo _binary_apps[];

void init_sched(void) {

    // Main user process
    struct Proc * proc_shell = get_first_free_proc();
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_PROC_A]);
    load_create_process_user(proc_shell, &_binary_apps[APP_IND_SHELL], DEF_ARGV);
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_FILESYSTEM], DEF_ARGV);
    
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_TESTS_SHELL], DEF_ARGV);

    // Extra initial processes....
    // load_create_process_user(get_first_free_proc(), 
    //                     &_binary_apps[APP_IND_FILESYSTEM], DEF_ARGV);

    
    printf("AT CREATE PROCESS SHELL expected pc= %x, ", (uint32_t)VADDR_USER_BASE);
    printProc(proc_shell);

    
    // Start proc_shell!
    switch_proc(proc_shell);
    
    PANIC("unreachable here!");
}


#else // IS X86



extern char __user_proc_a_start[], __user_proc_a_end[];
extern char __user_proc_b_start[], __user_proc_b_end[];

void init_sched(void) {

    struct AppBinaryInfo proc_a_app = {
        .start = __user_proc_a_start,
        .size  = (size_t)(__user_proc_a_end - __user_proc_a_start),
    };
    printf("[INIT SCHED] MOCKED PROC A APP physical addr start= 0x%x, end 0x%x \n", __user_proc_a_start, __user_proc_a_end);

    struct AppBinaryInfo proc_b_app = {
        .start = __user_proc_b_start,
        .size  = (size_t)(__user_proc_b_end - __user_proc_b_start),
    };
    printf("[INIT SCHED] MOCKED PROC B APP physical addr start= 0x%x, end 0x%x \n", __user_proc_b_start, __user_proc_b_end);

    struct Proc * proc_a = get_first_free_proc();
    struct Proc * proc_b = get_first_free_proc();

    load_create_process_user(proc_a, &proc_a_app, DEF_ARGV);
    load_create_process_user(proc_b, &proc_b_app, DEF_ARGV);
    
    printf("[INIT SCHED] PROC A created: ");
    printProc(proc_a);
    
    printf("[INIT SCHED] PROC B created: ");
    printProc(proc_b);

    printf("START!\n");
    
    switch_proc(proc_a);

    //set_curr(proc_a);
    //switch_page_table(curr->page_table, &curr->stack[sizeof(curr->stack)]);
    //proc_a_entry();

    PANIC("unreachable here!");
}

#endif