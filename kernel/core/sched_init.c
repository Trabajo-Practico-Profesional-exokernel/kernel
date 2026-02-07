#include "sched.h"
#include "proc.h"
#include "arch/logging.h"
#include "arch/mem_layout.h"
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "constants.h"
#include "arch/cpus.h"
#include "stdio.h"
#include "console/debug.h"
#include "console_files.h"
#include "proc_disk_loading.h"

char *DEF_ARGV[] = { "sh_prog","parameter1", 0 };
char *DEF_FS_ARGS[] = { "filesystem", 0 };
char *DEF_SHELL_ARGS[] = { "shell", 0 };


char*APP_NAME_SHELL = "shell";

char*APP_NAME_FILESYSTEM = "filesystem";
char*APP_NAME_COORDINATOR = "coordinator";
char*APP_NAME_SIMPLE_FRK = "simple_fork";

char*APP_NAME_PROC_A = "proc_a";
char*APP_NAME_PERIODIC_YIELD = "periodic_yield";



struct Proc * create_process_from_ind(int ind, char ** argv){
    struct Proc * proc = get_first_free_proc();
    init_proc_std_files(proc->pid);
    proc->gid = 0;

    struct BinaryAppEntry* app = get_app_from_ind(ind);
    if(app == NULL){
        PANIC("Invalid app ind %d to create process!", ind);
    }
    
    load_create_process_user(proc, app, argv);
    return proc;
}

struct Proc * create_process(char* proc_name, char ** argv){
    int ind = get_app_from_name(proc_name);

    if(ind < 0){
        PANIC("Invalid app name %s to create process!", proc_name);
    }

    return create_process_from_ind(ind, argv);
}
void init_sched2(void) {

    // Main user process
    // load_create_process_user(proc_shell, &_binary_apps[APP_NAME_PROC_A]);

    #ifdef IS_RISC
    struct Proc * proc_fs = create_process(APP_NAME_FILESYSTEM, DEF_ARGV);
    filesystem_PID = proc_fs->pid;

    struct Proc * proc_shell = create_process(APP_NAME_SHELL, DEF_ARGV);
    #else
    // load_create_process_user(proc_shell, &_binary_apps[APP_NAME_SHELL], DEF_ARGV);
    struct Proc * proc_shell = create_process(APP_NAME_SHELL, DEF_ARGV);
//    struct Proc * fs_server = create_process(APP_NAME_FILESYSTEM, DEF_ARGV);
//    filesystem_PID = fs_server->pid;
    // create_process(APP_NAME_PROC_A, DEF_ARGV);
    // create_process(APP_NAME_PERIODIC_YIELD, DEF_ARGV);

    #endif
    // load_create_process_user(proc_shell, &_binary_apps[APP_NAME_FILESYSTEM], DEF_ARGV);
    
    // load_create_process_user(proc_shell, &_binary_apps[APP_NAME_TESTS_SHELL], DEF_ARGV);

    // Extra initial processes....
    // load_create_process_user(get_first_free_proc(), 
    //                     &_binary_apps[APP_NAME_FILESYSTEM], DEF_ARGV);

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

    #ifdef IS_RISC
    struct Proc * proc_def = create_process(APP_NAME_COORDINATOR, DEF_ARGV);
    coordinator_PID = proc_def->pid;
    // struct Proc * proc_def = create_process(APP_NAME_SIMPLE_FRK, DEF_ARGV);
    // coordinator_PID = proc_def->pid;

    switch_proc(proc_def);

    #else

    // struct Proc * proc_fs = create_process(APP_NAME_FILESYSTEM, DEF_FS_ARGS);
    struct Proc * proc_shell = create_process(APP_NAME_SHELL, DEF_SHELL_ARGS);
    // struct Proc * proc_shell = create_process(APP_NAME_SIMPLE_FRK, DEF_SHELL_ARGS);

    // switch_proc(proc_fs);
    switch_proc(proc_shell);
    #endif
    
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

        printf("INITING IDLE PROC In case all procs are blocked!\n");        
        init_idle_proc();
        
        printf("INITING USER APP HEADERS!\n");        
        init_proc_headers();

        printf("Main NOW RELEASE!\n");        
        // for (int i = 0; i < 1000000000; i++){}
    } else {
        printf("Secondary acquired lock!?! %d.. not main == %d .. name '%s'\n", cpuid(), main_cpuid, lock_test.name);        
        // for (int i = 0; i < 1000000000; i++){}
        printf("Secondary NOW RELEASE!\n");        
    }

    release(&lock_test);
    // for (int i = 0; i < 800000000; i++){}
    
    acquire(&lock_test);

    printf("Just one cpu should init sched %d == main == %d?!\n", cpuid(), main_cpuid);

    
    struct Proc * proc_shell = create_process(APP_NAME_SHELL, DEF_SHELL_ARGS);
    switch_proc(proc_shell);

    // init_sched_main();
}
