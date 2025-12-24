#include "arch/trap_handling.h"
#include "inc/common.h"

// Sched exec , wait and so on...
#include "sched.h"
#include "proc_syscalls.h"
#include "proc.h"
#include "arch/mem.h" // needed for switch to kernel page tables
#include "arch_inc/trap_constants.h"
#include "arch/logging.h"
#include "std/string.h"

#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  
extern struct AppBinaryInfo _binary_apps[];


struct ProcExitStatus exit_statuses[PROCS_MAX]; // Have for every process a current return status. 






void reset_exit_status(struct ProcExitStatus* status){
    status->ret_code = 0;
    status->waiters_head = NULL;
    status->waiters_tail = NULL;
    
    status->link_next_waiting = NULL;
}


// Simple linked queue push
void add_waiter_for(struct ProcExitStatus* exited_status, struct ProcExitStatus* waiter){

    if(exited_status->waiters_tail == NULL){ // First waiter == its both head and tail
        // Basically its resetting to a 1 element waiter. i.e head == tail == waiter
        exited_status->waiters_head = waiter;
        exited_status->waiters_tail = waiter;
        
        waiter->link_next_waiting = NULL;
        return;
    }

    exited_status->waiters_tail->link_next_waiting = waiter;
    exited_status->waiters_tail = waiter;
}


void notify_exited(struct ProcExitStatus* exited_status, int ret_code){
    struct ProcExitStatus* waiter = exited_status->waiters_head;

    while(waiter != NULL){
        struct Proc* waiter_proc = get_proc(waiter->proc_pid);

        if(waiter_proc != NULL && waiter_proc->status == PROC_NOT_RUNNABLE){
            struct TrapFrame * proc_tf = &waiter_proc->tf;
            SET_SYSCALL_RET0(proc_tf, ret_code);
            waiter_proc->status = PROC_RUNNABLE; // Now is runnable! So scheduler might start it again.
        }

        waiter = waiter->link_next_waiting;
    }
    
    reset_exit_status(exited_status);
}



void syscall_exec(FullTrapFrame *tf, uintptr_t pc) {

    int prog_ind = SYSCALL_ARG0(tf);
    if (prog_ind < 0 || prog_ind>= APP_COUNT){
        printf("Invalid exec call ind %d \n", prog_ind);
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
        printf("NO proc params exec\n");
        argv_pointers[0] = 0;
    }

    printf("Should run program at ind %d \n", prog_ind);

    //int set_init_parameters_for_proc(struct Proc * proc, char ** argv, paddr_t* sp_out);
    
    struct Proc* proc= get_first_free_proc();
    // It cannot but NULL it throws panic for now but check it anyway for the future!
    if (proc == NULL){
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)
        save_curr_proc_state(tf, pc + 4);    
        sched_yield();
        return;
    }

    printf("Should run free proc %p binary: %p \n", proc, &_binary_apps[prog_ind]);

    #ifdef IS_RISC
    switch_to_kernel_tables();
    load_create_process_user(proc, &_binary_apps[prog_ind], (char **) &argv_pointers[0]);

    reset_exit_status(&exit_statuses[PROCX(proc->pid)]);

    // Do switch to new proc? ... no?
    SET_SYSCALL_RET0(tf, proc->pid)


    // When a new process is to be executed, you reduce response time by running it first.
    // Save parent proc state
    save_curr_proc_state(tf, pc + 4);
    parent_proc->status = PROC_RUNNABLE;
    
    switch_proc(proc);


    // Another alternative would be to go back to parent proc!
    // switch_page_table((uint32_t *)(get_curr()->pde_paddr));
    #else

    load_create_process_user(proc, &_binary_apps[prog_ind], (char **) &argv_pointers[0]);
    // Now do switch? or not? naaa If you want you could wait for it! after ret.
    SET_SYSCALL_RET0(tf, proc->pid)
    #endif

}


void syscall_exit(FullTrapFrame *tf, uintptr_t pc){
    int exit_code = SYSCALL_ARG0(tf);
    struct Proc * exited_proc = get_curr();
    
    printf("Process %u exited with code %d ", exited_proc->pid, exit_code);

    #ifdef IS_RISC
    switch_to_kernel_tables();
    #endif
    
    struct ProcExitStatus* exit_status = &exit_statuses[PROCX(exited_proc->pid)];

    if (exit_status->waiters_head == NULL){
        printf("had no waiters.. orphan process until awaited\n");
        // No waiter means, orphan proc until somebody waits it?
        exited_proc->status = PROC_DYING;
        exit_status->ret_code = exit_code;

        sched_yield();
        return; // Not reachable but for clarity
    }
    printf("had waiters.. notified and free\n");

    notify_exited(exit_status, exit_code);

    // Delete! .. from proc.c
    free_process(exited_proc);

    // TO DO! Notify for processes waiting?
    sched_yield();
}


void syscall_wait(FullTrapFrame *tf, uintptr_t pc){
    procid_t waited_proc_pid = SYSCALL_ARG0(tf);
    struct Proc * waiter_proc = get_curr();
    printf("Process %d should wait at pc: %x(ret to %x) for %d: ",waiter_proc->pid, pc, pc+4, waited_proc_pid);

    struct Proc* waited_proc = get_proc(waited_proc_pid);

    if(waited_proc == NULL || waited_proc->status == PROC_FREE){
        printf("Error waited proc was non valid, or was on a invalid state\n");
        SET_SYSCALL_RET0(tf, -1) // Error
        return;
    }
    struct ProcExitStatus* waited_exit_status = &exit_statuses[PROCX(waited_proc_pid)];
    if(waited_proc->status == PROC_DYING){
        printf("already exited, cleaning orphan and returning to waiter!\n");
        // Already finished! So notify directly and return to curr process? no need for sched yield
        SET_SYSCALL_RET0(tf, waited_exit_status->ret_code)
        
        reset_exit_status(waited_exit_status);
        free_process(waited_proc);
        
        return;
    }

    struct ProcExitStatus* waiter_exit_status = &exit_statuses[PROCX(waiter_proc->pid)];
    printf("not exited yet, wait blocked!\n");

    waiter_proc->status = PROC_NOT_RUNNABLE;
    add_waiter_for(waited_exit_status, waiter_exit_status);
    
    save_curr_proc_state(tf, pc + 4);
    sched_yield();    
}


void syscall_yield(FullTrapFrame *tf, uintptr_t pc){
    save_curr_proc_state(tf, pc);
    sched_yield();
}


void init_syscalls_proc(void) {
    register_syscall(SYS_EXEC, syscall_exec);
    register_syscall(SYS_EXIT, syscall_exit);
    register_syscall(SYS_WAIT, syscall_wait);
    register_syscall(SYS_YIELD, syscall_yield);
}