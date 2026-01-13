#include "sched.h"
#include "proc.h"
#include "inc/common.h"
#include "fd.h"
#include "arch/logging.h"
#include "arch/mem_layout.h"
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  


char *DEF_ARGV[] = { "sh_prog","parameter1", 0 };

extern struct AppBinaryInfo _binary_apps[];

struct Proc * create_process(size_t ind, char ** argv){
    struct Proc * proc = get_first_free_proc();
    load_create_process_user(proc, &_binary_apps[ind], argv);
    return proc;
}

void init_sched(void) {

    // Main user process
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_PROC_A]);

    #ifdef IS_RISC
    /*struct Proc * proc_fs = create_process(APP_IND_FILESYSTEM, DEF_ARGV);
    filesystem_PID = proc_fs->pid;

    struct Proc * proc_shell = create_process(APP_IND_SHELL, DEF_ARGV);*/
    struct Proc * proc_a = create_process(APP_IND_PROC_A, DEF_ARGV);
    #else
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_SHELL], DEF_ARGV);
    struct Proc * proc_shell = create_process(APP_IND_PROC_A, DEF_ARGV);

    // create_process(APP_IND_PROC_A, DEF_ARGV);
    // create_process(APP_IND_PERIODIC_YIELD, DEF_ARGV);

    #endif
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_FILESYSTEM], DEF_ARGV);
    
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_TESTS_SHELL], DEF_ARGV);

    // Extra initial processes....
    // load_create_process_user(get_first_free_proc(), 
    //                     &_binary_apps[APP_IND_FILESYSTEM], DEF_ARGV);

    
    debug_printf("AT CREATE PROCESS SHELL expected pc= %x, ", (uint32_t)VADDR_USER_BASE);
    //printProc(proc_shell);

    
    // Start proc_shell!
    #ifdef IS_RISC
    switch_proc(proc_a);
    #else
    switch_proc(proc_shell);
    #endif
    
    PANIC("unreachable here!");
}

void init_sched2(void) {
    struct Proc * proc_def = create_process(APP_IND_KALLOC_PROGRAM, DEF_ARGV);
    debug_printf("AT CREATE PROCESS DEF expected pc= %x, ", (uint32_t)VADDR_USER_BASE);
    printProc(proc_def);
    
    switch_proc(proc_def);
    
}