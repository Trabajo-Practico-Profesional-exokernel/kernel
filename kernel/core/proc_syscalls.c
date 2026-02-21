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
#include "console_files.h"
#include "stdio.h"
#include "console/debug.h"
#include "arch/ipc.h"

#include "proc_sleeping.h"


struct ProcExitStatus exit_statuses[PROCS_MAX]; // Have for every process a current return status. 

void reset_exit_status(struct ProcExitStatus* status){
    status->ret_code = 0;
    status->waiters_head = NULL;
    status->waiters_tail = NULL;
    
    status->link_next_waiting = NULL;
}

struct ProcExitStatus* get_exit_status(int pid){
    return &exit_statuses[PROCX(pid)];
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

void syscall_exit(FullTrapFrame *tf, uintptr_t pc){
    int exit_code = SYSCALL_ARG0(tf);
    struct Proc * exited_proc = myproc();
    
    VERBOSE_DEBUG_PRINTF("Process %u exited with code %d ", exited_proc->pid, exit_code);

    switch_to_kernel_tables();
    
    struct ProcExitStatus* exit_status = &exit_statuses[PROCX(exited_proc->pid)];

    if (exit_status->waiters_head == NULL){
        VERBOSE_DEBUG_PRINTF("had no waiters.. orphan process until awaited\n");
        // No waiter means, orphan proc until somebody waits it?
        exited_proc->status = PROC_DYING;
        exit_status->ret_code = exit_code;

        sched_yield();
        return; // Not reachable but for clarity
    }
    VERBOSE_DEBUG_PRINTF("had waiters.. notified and free\n");

    notify_exited(exit_status, exit_code);

    // Delete! .. from proc.c
    free_process(exited_proc);

    // TO DO! Notify for processes waiting?
    sched_yield();
}

void syscall_wait(FullTrapFrame *tf, uintptr_t pc){
    procid_t waited_proc_pid = SYSCALL_ARG0(tf);
    struct Proc * waiter_proc = myproc();
    VERBOSE_DEBUG_PRINTF("Process %d should wait at pc: %x(ret to %x) for %d: ",waiter_proc->pid, pc, pc+4, waited_proc_pid);

    struct Proc* waited_proc = get_proc(waited_proc_pid);

    if(waited_proc == NULL || waited_proc->status == PROC_FREE){
        VERBOSE_DEBUG_PRINTF("Error waited proc was non valid, or was on a invalid state\n");
        SET_SYSCALL_RET0(tf, -1) // Error
        return;
    }
    struct ProcExitStatus* waited_exit_status = &exit_statuses[PROCX(waited_proc_pid)];
    
    if(waited_proc->status == PROC_DYING){
        VERBOSE_DEBUG_PRINTF("already exited, cleaning orphan and returning to waiter!\n");
        
        switch_to_kernel_tables();
        // Already finished! So notify directly and return to curr process? no need for sched yield
        SET_SYSCALL_RET0(tf, waited_exit_status->ret_code)
        
        reset_exit_status(waited_exit_status);
        free_process(waited_proc);
        
        switch_page_table((uint32_t *) waiter_proc->pde_paddr);
        
        return;
    }

    struct ProcExitStatus* waiter_exit_status = &exit_statuses[PROCX(waiter_proc->pid)];
    VERBOSE_DEBUG_PRINTF("not exited yet, wait blocked!\n");

    waiter_proc->status = PROC_NOT_RUNNABLE;
    add_waiter_for(waited_exit_status, waiter_exit_status);
    
    save_curr_proc_state(tf, pc + 4);
    sched_yield();    
}

void syscall_kill(FullTrapFrame *tf, uintptr_t pc){
    procid_t killed_proc_pid = SYSCALL_ARG0(tf);
    struct Proc * killer_proc = myproc();
    VERBOSE_DEBUG_PRINTF("Process %d should kill at pc: %x(ret to %x) for %d: ",killer_proc->pid, pc, pc+4, killed_proc_pid);

    struct Proc* killed_proc = get_proc(killed_proc_pid);

    if(killed_proc == NULL || killed_proc->status == PROC_FREE){
        VERBOSE_PRINTF("Error waited proc was non valid, or was on a invalid state\n");
        SET_SYSCALL_RET0(tf, -1) // Error
        return;
    }
    struct ProcExitStatus* killed_exit_status = &exit_statuses[PROCX(killed_proc_pid)];
    
    if(killed_proc->status == PROC_DYING){
        VERBOSE_DEBUG_PRINTF(" already exited, cleaning orphan and returning to waiter!\n");
        
        switch_to_kernel_tables();
        // Already finished! So notify directly and return to curr process? no need for sched yield
        SET_SYSCALL_RET0(tf, killed_exit_status->ret_code)
        
        reset_exit_status(killed_exit_status);
        free_process(killed_proc);
        
        switch_page_table((uint32_t *) killer_proc->pde_paddr);
        
        return;
    }
    VERBOSE_DEBUG_PRINTF(" did not exit... cleanup/forcefully!\n");
    notify_exited(killed_exit_status, -3); // Code for forcefully exited?
    
    switch_to_kernel_tables();
    // Already finished! So notify directly and return to curr process? no need for sched yield
    SET_SYSCALL_RET0(tf, 0)
    
    reset_exit_status(killed_exit_status);
    free_process(killed_proc);
    
    switch_page_table((uint32_t *) killer_proc->pde_paddr);

}


void syscall_yield(FullTrapFrame *tf, uintptr_t pc){
    save_curr_proc_state(tf, pc+ 4);
    sched_yield();
}


void syscall_getpid(FullTrapFrame *tf, uintptr_t pc){
    struct Proc * curr_proc = myproc();
    int pid = curr_proc->pid;
    SET_SYSCALL_RET0(tf, pid);
}

void syscall_get_coord_pid(FullTrapFrame *tf, uintptr_t pc){
    SET_SYSCALL_RET0(tf, coordinator_PID);
}

void syscall_alive(FullTrapFrame *tf, uintptr_t pc){
    uint32_t pid = SYSCALL_ARG0(tf);
    struct Proc *proc = get_proc(pid);
    uint32_t is_alive = proc->status != PROC_DYING && proc->status != PROC_FREE; 
    SET_SYSCALL_RET0(tf, is_alive);
}


void syscall_virtual_copy(FullTrapFrame *tf, uintptr_t pc){

    uint32_t src_vaddr = SYSCALL_ARG0(tf);
    uint32_t dst_vaddr = SYSCALL_ARG1(tf);
    uint32_t len = SYSCALL_ARG2(tf);
    uint32_t src_pid = SYSCALL_ARG3(tf);

    switch_to_kernel_tables();

    uint32_t src_paddr = get_paddr_for((uint32_t*)get_proc(src_pid)->pde_paddr, src_vaddr);
    uint32_t dst_paddr = get_paddr_for((uint32_t*)myproc()->pde_paddr, dst_vaddr);

    if (src_paddr != 0 && dst_paddr != 0) {
        memcpy((void *)dst_paddr, (void *)src_paddr, len);
        SET_SYSCALL_RET0(tf, SUCCESS);
    } else {
        SET_SYSCALL_RET0(tf, ERROR); 
    }

    switch_page_table((uint32_t *) myproc()->pde_paddr);
}









void init_syscalls_ipc(void){
    register_syscall(SYS_TRY_SEND_CONTENT, syscall_try_send_content);
    register_syscall(SYS_TRY_RECV_CONTENT, syscall_try_recv_content);
    register_syscall(SYS_RECV_CONTENT, syscall_recv_content);
}

void init_syscalls_proc(void) {

    for(int i =0; i < PROCS_MAX; i++){
        exit_statuses[i].proc_pid = i;
    }
    register_syscall(SYS_EXEC, syscall_exec);
    register_syscall(SYS_PROC_EXECV, syscall_execv);
    
    register_syscall(SYS_FORK, syscall_fork);

    register_syscall(SYS_EXIT, syscall_exit);
    register_syscall(SYS_WAIT, syscall_wait);
    register_syscall(SYS_KILL, syscall_kill);

    register_syscall(SYS_YIELD, syscall_yield);
    register_syscall(SYS_GETPID, syscall_getpid);
    register_syscall(SYS_COORDPID, syscall_get_coord_pid);
    register_syscall(SYS_ALIVE, syscall_alive);
    register_syscall(SYS_VIRTUAL_COPY, syscall_virtual_copy);

    register_syscall(SYS_SLEEP, syscall_sleep);
    register_syscall(SYS_UPTIME, syscall_uptime);

    // register_syscall(SYS_LOCK_SLEEP, syscall_lock_sleep);

}

