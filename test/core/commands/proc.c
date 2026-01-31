#include "sched.h"
#include "proc.h"
#include "fd.h"
#include "arch/logging.h"
#include "arch/mem_layout.h"
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  
#include "constants.h"
#include "arch/cpus.h"
#include "stdio.h"
#include "console/debug.h"

#include "interactive_test_commands.h"

extern struct AppBinaryInfo _binary_apps[];

struct Proc * create_process_test(size_t ind, char ** argv){
    struct Proc * proc = get_first_free_proc();
    proc->gid = 0;
    load_create_process_user(proc, &_binary_apps[ind], argv);
    return proc;
}

char *DEF_ARGV_TEST[] = { "sh_prog","parameter1", 0 };


void start_shell(void){
    struct Proc * proc_def = create_process_test(APP_IND_COORDINATOR, DEF_ARGV_TEST);
    coordinator_PID = proc_def->pid;
    //create_process(APP_IND_SHELL, DEF_ARGV);
    //struct Proc * proc_def = create_process(APP_IND_PROC_A, DEF_ARGV);
    //struct Proc * proc_def_2 = create_process(APP_IND_PROC_B, DEF_ARGV);
    switch_proc(proc_def);

}