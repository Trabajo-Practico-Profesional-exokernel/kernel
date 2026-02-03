#include "arch/trap_handling.h"
#include "constants.h"

// Sched exec , wait and so on...
#include "sched.h"
#include "proc_syscalls.h"
#include "proc.h"
#include "arch/mem.h" // needed for switch to kernel page tables
#include "arch_inc/trap_constants.h"
#include "arch/logging.h"
#include "string.h"
#include "stdlib.h"

#include "stdio.h"
#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  
#include "console/debug.h"
#include "arch/ipc.h"

extern struct AppBinaryInfo _binary_apps[];

void syscall_exec(FullTrapFrame *tf, uintptr_t pc) {

    int prog_ind = SYSCALL_ARG0(tf);
    if (prog_ind < 0 || prog_ind>= APP_COUNT){
        debug_printf("Invalid exec call ind %d \n", prog_ind);
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)        
        return;
    }
    vaddr_t vaddr_argv_pointer = SYSCALL_ARG1(tf);
    struct Proc* parent_proc = get_curr();
    switch_to_kernel_tables();
    
    paddr_t argv_pointers[MAXARG]; 
    
    if (vaddr_argv_pointer != 0){
        int argc = copy_argv_pointers_from_user(parent_proc, &argv_pointers[0], vaddr_argv_pointer);

        if (argc< 0){
            SET_SYSCALL_RET0(tf, argc)
            save_curr_proc_state(tf, pc + 4);    
            sched_yield();
            return;
        }
    } else {
        debug_printf("NO proc params exec\n");
        argv_pointers[0] = 0;
    }

    debug_printf("Should run program at ind %d \n", prog_ind);

    //int set_init_parameters_for_proc(struct Proc * proc, char ** argv, paddr_t* sp_out);
    
    struct Proc* proc= get_first_free_proc();
    // It cannot but NULL it throws panic for now but check it anyway for the future!
    if (proc == NULL){
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)
        save_curr_proc_state(tf, pc + 4);    
        sched_yield();
        return;
    }

    debug_printf("Should run free proc %p binary: %p \n", proc, &_binary_apps[prog_ind]);

    load_create_process_user(proc, &_binary_apps[prog_ind], (char **) &argv_pointers[0]);

    debug_printf("Loaded proc\n");
    reset_exit_status(get_exit_status(proc->pid));

    // Do switch to new proc? ... no?
    SET_SYSCALL_RET0(tf, proc->pid)


    // When a new process is to be executed, you reduce response time by running it first.
    // Save parent proc state
    save_curr_proc_state(tf, pc + 4);
    parent_proc->status = PROC_RUNNABLE;
    
    switch_proc(proc);

}




void syscall_fork(FullTrapFrame *tf, uintptr_t pc) {

    struct Proc* parent_proc = get_curr();
    switch_to_kernel_tables();
    
    printf("Should fork program at ind %d \n", parent_proc->pid);
    
    struct Proc* child_proc= get_first_free_proc();

    if (child_proc == NULL){
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)
        save_curr_proc_state(tf, pc + 4);    
        
        switch_page_table((uint32_t *) parent_proc->pde_paddr);
        return;
    }

    uintptr_t trg_init_pc = pc + 4;
    printf("Forked?\n");
    
    SET_SYSCALL_RET0(tf, child_proc->pid)
    save_curr_proc_state(tf, trg_init_pc);

    int ret = load_create_forked(parent_proc, child_proc);

    if(ret < 0){
        SET_SYSCALL_RET0(tf, ret);
        save_curr_proc_state(tf, trg_init_pc);
    } else {
        reset_exit_status(get_exit_status(child_proc->pid));
    }


    switch_page_table((uint32_t *) parent_proc->pde_paddr);
}
