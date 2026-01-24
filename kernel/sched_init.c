#include "sched.h"
#include "proc.h"
#include "inc/common.h"
#include "fd.h"
#include "arch/logging.h"
#include "arch/mem_layout.h"
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  

<<<<<<< HEAD
#include "arch/cpus.h"

=======
// #include "user_pages_alloc.h" 
>>>>>>> clear_mem_manage

char *DEF_ARGV[] = { "sh_prog","parameter1", 0 };

extern struct AppBinaryInfo _binary_apps[];

struct Proc * create_process(size_t ind, char ** argv){
    struct Proc * proc = get_first_free_proc();
    proc->gid = 0;
    load_create_process_user(proc, &_binary_apps[ind], argv);
    return proc;
}

void init_sched2(void) {

    // Main user process
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_PROC_A]);

    #ifdef IS_RISC
    struct Proc * proc_fs = create_process(APP_IND_FILESYSTEM, DEF_ARGV);
    filesystem_PID = proc_fs->pid;

    struct Proc * proc_shell = create_process(APP_IND_SHELL, DEF_ARGV);
    #else
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_SHELL], DEF_ARGV);
    struct Proc * proc_shell = create_process(APP_IND_SHELL, DEF_ARGV);
//    struct Proc * fs_server = create_process(APP_IND_FILESYSTEM, DEF_ARGV);
//    filesystem_PID = fs_server->pid;
    // create_process(APP_IND_PROC_A, DEF_ARGV);
    // create_process(APP_IND_PERIODIC_YIELD, DEF_ARGV);

    #endif
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_FILESYSTEM], DEF_ARGV);
    
    // load_create_process_user(proc_shell, &_binary_apps[APP_IND_TESTS_SHELL], DEF_ARGV);

    // Extra initial processes....
    // load_create_process_user(get_first_free_proc(), 
    //                     &_binary_apps[APP_IND_FILESYSTEM], DEF_ARGV);

    copy_pages_code_segment(proc_shell, proc_shell);
    debug_printf("AT CREATE PROCESS SHELL expected pc= %x, ", (uint32_t)VADDR_USER_BASE);
    //printProc(proc_shell);

    
    // Start proc_shell!
    #ifdef IS_RISC
    switch_proc(proc_fs);
    #else
    switch_proc(proc_shell);
    #endif
    
    PANIC("unreachable here!");
}

void init_sched_main(void) {
//    struct Proc * proc_def = create_process(APP_IND_PROC_A, DEF_ARGV);
//    debug_printf("AT CREATE PROCESS DEF expected pc= %x, ", (uint32_t)VADDR_USER_BASE);
//    printProc(proc_def);
    struct Proc * proc_def = create_process(APP_IND_FILESYSTEM, DEF_ARGV);
    create_process(APP_IND_SHELL, DEF_ARGV);

    switch_proc(proc_def);
    
}

#include "arch/spin_locks.h"

struct spinlock lock_test;
int main_cpuid = -1;

void init_sched(void) {

    acquire(&lock_test);
    printf("ACQUIRED ?!\n");

    // for (int i = 0; i < 2000000000; i++){}

    printf("AFTER SLEEP ?!\n");
    
    if(main_cpuid < 0){
        main_cpuid = cpuid();
        printf("Main cpu acquired lock!?! %d\n",main_cpuid);        
        lock_test.name= "main lock";

        // for (int i = 0; i < 1000000000; i++){}
        printf("Main NOW RELEASE!\n");        
    } else {
        printf("Secondary acquired lock!?! %d.. not main == %d .. name '%s'\n", cpuid(), main_cpuid, lock_test.name);        
        // for (int i = 0; i < 1000000000; i++){}
        printf("Secondary NOW RELEASE!\n");        
    }

    release(&lock_test);
    // for (int i = 0; i < 800000000; i++){}
    
    acquire(&lock_test);

    printf("Just one cpu should init sched %d == main == %d?!\n", cpuid(), main_cpuid);
    init_sched_main();
}
