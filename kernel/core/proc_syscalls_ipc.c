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



void syscall_try_send_content(FullTrapFrame *tf, uintptr_t pc){
    struct Proc * sender_proc = get_curr();
    int receiver_proc_pid = SYSCALL_ARG0(tf);
    uint32_t content_addr = SYSCALL_ARG1(tf);
    int len_content = SYSCALL_ARG2(tf);

    switch_to_kernel_tables();
    
    int result = send_content(sender_proc->pid, receiver_proc_pid, content_addr, len_content);

    SET_SYSCALL_RET0(tf, result);
    switch_page_table((uint32_t *) sender_proc->pde_paddr);
}

void syscall_try_recv_content(FullTrapFrame *tf, uintptr_t pc) {
    uint32_t content_addr = SYSCALL_ARG0(tf);
    uint32_t len_content = SYSCALL_ARG1(tf);
    uint32_t receiver_proc_pid = get_curr()->pid;
    
    switch_to_kernel_tables();

    int result = recv_content(receiver_proc_pid, content_addr, len_content);

    SET_SYSCALL_RET0(tf, result);

    switch_page_table((uint32_t *) get_curr()->pde_paddr);
}

void syscall_recv_content(FullTrapFrame *tf, uintptr_t pc) {

    uint32_t content_addr = SYSCALL_ARG0(tf);
    uint32_t len_content = SYSCALL_ARG1(tf);
    struct Proc *receiver_proc = get_curr();
    uint32_t receiver_proc_pid = receiver_proc->pid;
    
    switch_to_kernel_tables();

    int result = recv_content(receiver_proc_pid, content_addr, len_content);

    if (result == ERROR) { // Means no message available, so block
        receiver_proc->status = PROC_NOT_RUNNABLE;
        save_curr_proc_state(tf, pc);
        sched_yield();
    }

    SET_SYSCALL_RET0(tf, result);

    switch_page_table((uint32_t *) get_curr()->pde_paddr);

}
