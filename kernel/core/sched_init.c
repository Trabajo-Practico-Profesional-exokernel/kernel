#include "sched.h"
#include "proc.h"
#include "arch/logging.h"
#include "arch/mem_layout.h"
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch_inc/trap_constants.h" //defines perms like PAGE_R and so on.
#include "constants.h"
#include "arch/cpus.h"
#include "stdio.h"
#include "console/debug.h"
#include "console_files.h"
#include "proc_disk_loading.h"

#include "arch/arch_init.h"

char *DEF_ARGV[] = { "sh_prog","parameter1", 0 };
char *DEF_FS_ARGS[] = { "filesystem", 0 };
char *DEF_SHELL_ARGS[] = { "shell", 0 };

char *INF_LOOP_1_ARGS[] = { "infinite_loop", "Some LOG", 0 };
char *INF_LOOP_2_ARGS[] = { "infinite_loop", "LOG 2", 0 };


char*APP_NAME_SHELL = "shell";

char *APP_NAME_FILESYSTEM = "filesystem";
char *APP_NAME_COORDINATOR = "coordinator";
char *APP_NAME_SIMPLE_FRK = "simple_fork";

char *APP_NAME_PROC_A = "proc_a";
char *APP_NAME_PERIODIC_YIELD = "periodic_yield";



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


#include "arch/spin_locks.h"

struct spinlock lock_test;

void init_sched_secondary_cpu(void){
    init_cpu_info();
    printf("Secondary cpu %d should init sched secondary\n", cpuid());
    acquire(&lock_test);

    printf("Secondary CPU inited!! %d \n", cpuid());
    
    enable_timer_interrupts();
    sched_yield();

    release(&lock_test);
}

void init_sched_main_cpu(void) {
    init_cpu_info();
    set_as_main_cpu();

    lock_test.name= "main lock";
    acquire(&lock_test);

    start_secondary_cpus();

    printf("Just one cpu should init sched main %d?!\n", cpuid());


    #ifdef IS_RISC
    // struct Proc * first_main_proc = create_process("infinite_loop", INF_LOOP_1_ARGS);
    struct Proc * first_main_proc = create_process("malloc_program", DEF_ARGV);
    // struct Proc * first_main_proc = create_process(APP_NAME_COORDINATOR, DEF_ARGV);
    // coordinator_PID = first_main_proc->pid;
    #else
    // struct Proc * first_main_proc = create_process("infinite_loop", INF_LOOP_1_ARGS);
    // create_process("infinite_loop", INF_LOOP_2_ARGS);

    struct Proc * first_main_proc = create_process(APP_NAME_COORDINATOR, DEF_ARGV);
    coordinator_PID = first_main_proc->pid;
    #endif    

    release(&lock_test);
    enable_timer_interrupts();
    sched_yield();

    // switch_proc(first_main_proc);

}
