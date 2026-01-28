#include "arch/trap_handling.h"
#include "constants.h"

#include "fd.h"
// Sched exec , wait and so on...
#include "sched.h"
#include "proc_syscalls.h"
#include "proc.h"
#include "arch/mem.h" // needed for switch to kernel page tables
#include "arch_inc/trap_constants.h"
#include "arch/logging.h"
#include "string.h"
#include "stdlib.h"
#include "arch/communication.h"
#include "stdio.h"
#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  
#include "console/debug.h"
#include "arch/ipc.h"

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
    reset_exit_status(&exit_statuses[PROCX(proc->pid)]);

    // Do switch to new proc? ... no?
    SET_SYSCALL_RET0(tf, proc->pid)


    // When a new process is to be executed, you reduce response time by running it first.
    // Save parent proc state
    save_curr_proc_state(tf, pc + 4);
    parent_proc->status = PROC_RUNNABLE;
    
    switch_proc(proc);

}


void syscall_exit(FullTrapFrame *tf, uintptr_t pc){
    int exit_code = SYSCALL_ARG0(tf);
    struct Proc * exited_proc = get_curr();
    
    debug_printf("Process %u exited with code %d ", exited_proc->pid, exit_code);

    switch_to_kernel_tables();
    
    struct ProcExitStatus* exit_status = &exit_statuses[PROCX(exited_proc->pid)];

    if (exit_status->waiters_head == NULL){
        debug_printf("had no waiters.. orphan process until awaited\n");
        // No waiter means, orphan proc until somebody waits it?
        exited_proc->status = PROC_DYING;
        exit_status->ret_code = exit_code;

        sched_yield();
        return; // Not reachable but for clarity
    }
    debug_printf("had waiters.. notified and free\n");

    notify_exited(exit_status, exit_code);

    // Delete! .. from proc.c
    free_process(exited_proc);

    // TO DO! Notify for processes waiting?
    sched_yield();
}


void syscall_wait(FullTrapFrame *tf, uintptr_t pc){
    procid_t waited_proc_pid = SYSCALL_ARG0(tf);
    struct Proc * waiter_proc = get_curr();
    debug_printf("Process %d should wait at pc: %x(ret to %x) for %d: ",waiter_proc->pid, pc, pc+4, waited_proc_pid);

    struct Proc* waited_proc = get_proc(waited_proc_pid);

    if(waited_proc == NULL || waited_proc->status == PROC_FREE){
        debug_printf("Error waited proc was non valid, or was on a invalid state\n");
        SET_SYSCALL_RET0(tf, -1) // Error
        return;
    }
    struct ProcExitStatus* waited_exit_status = &exit_statuses[PROCX(waited_proc_pid)];
    if(waited_proc->status == PROC_DYING){
        debug_printf("already exited, cleaning orphan and returning to waiter!\n");
        // Already finished! So notify directly and return to curr process? no need for sched yield
        SET_SYSCALL_RET0(tf, waited_exit_status->ret_code)
        
        reset_exit_status(waited_exit_status);
        free_process(waited_proc);
        
        return;
    }

    struct ProcExitStatus* waiter_exit_status = &exit_statuses[PROCX(waiter_proc->pid)];
    debug_printf("not exited yet, wait blocked!\n");

    waiter_proc->status = PROC_NOT_RUNNABLE;
    add_waiter_for(waited_exit_status, waiter_exit_status);
    
    save_curr_proc_state(tf, pc + 4);
    sched_yield();    
}


void syscall_yield(FullTrapFrame *tf, uintptr_t pc){
    save_curr_proc_state(tf, pc+ 4);
    sched_yield();
}


void syscall_getpid(FullTrapFrame *tf, uintptr_t pc){
    struct Proc * curr_proc = get_curr();
    int pid = curr_proc->pid;
    SET_SYSCALL_RET0(tf, pid);
}

void syscall_get_coord_pid(FullTrapFrame *tf, uintptr_t pc){
    SET_SYSCALL_RET0(tf, coordinator_PID);
}

void syscall_trysendmsg(FullTrapFrame *tf, uintptr_t pc){
    debug_printf("syscall send_msg...\n");
    struct Proc * sender_proc = get_curr();
    int receiver_proc_pid = SYSCALL_ARG0(tf);
    uint32_t msg_addr = SYSCALL_ARG1(tf);
    int type_msg = SYSCALL_ARG2(tf);

    switch_to_kernel_tables();

    
    if (receiver_proc_pid == 99) {
        //receiver_proc_pid = filesystem_PID;
        receiver_proc_pid = 2; // TODO: change this, for now fs server is the first proc.
    }
    
    int result = send_msg(sender_proc, receiver_proc_pid, msg_addr, type_msg);

    // Switch back to proc tables to return to proc
    SET_SYSCALL_RET0(tf, result);
    switch_page_table((uint32_t *) sender_proc->pde_paddr);
    
}

void syscall_tryrecvmsg(FullTrapFrame *tf, uintptr_t pc) {
    debug_printf("syscall tryrecvmsg...\n");
    uint32_t msg_addr = SYSCALL_ARG0(tf);

    struct Proc *receiver_proc = get_curr();
    
    switch_to_kernel_tables();

    int result = recv_msg(receiver_proc, msg_addr);

    SET_SYSCALL_RET0(tf, result);

    switch_page_table((uint32_t *) receiver_proc->pde_paddr);
}

void syscall_recvmsg(FullTrapFrame *tf, uintptr_t pc) {
    debug_printf("syscall recvmsg...\n");
    uint32_t msg_addr = SYSCALL_ARG0(tf);

    struct Proc *receiver_proc = get_curr();

    switch_to_kernel_tables();

    int result = recv_msg(receiver_proc, msg_addr);
    
    if (result == -1) { // Means no message available! So block
        receiver_proc->status = PROC_NOT_RUNNABLE;
        save_curr_proc_state(tf, pc);
        sched_yield();
    }

    SET_SYSCALL_RET0(tf, result);
    switch_page_table((uint32_t *) receiver_proc->pde_paddr);
}

void syscall_pipe(FullTrapFrame *tf, uintptr_t pc) {
    //enable_debug_print();
    debug_printf("syscall pipe...\n");
    struct Proc * curr_proc = get_curr();
    uint32_t vaddr_pipe = SYSCALL_ARG0(tf);

    switch_to_kernel_tables();

    int *user_fds_ptr = (int *) get_paddr_for((paddr_t*)curr_proc->pde_paddr, vaddr_pipe);
    struct MemBuffer* buffer = membuffer_alloc();
    membuffer_reset(buffer);
    int fd_r = get_file_descriptor(curr_proc);
    curr_proc->files[fd_r] = alloc_file();
    fd_init(curr_proc->files[fd_r], FD_TYPE_PIPE, PERM_READ);

    int fd_w = get_file_descriptor(curr_proc);
    curr_proc->files[fd_w] = alloc_file();
    fd_init(curr_proc->files[fd_w], FD_TYPE_PIPE, PERM_WRITE);
    
    add_buffer_to_file(curr_proc->files[fd_r], buffer);
    add_buffer_to_file(curr_proc->files[fd_w], buffer);
    
    user_fds_ptr[0] = fd_r;
    user_fds_ptr[1] = fd_w;
    int success = 1;
    SET_SYSCALL_RET0(tf, success);
    disable_debug_print();
    
    switch_page_table((uint32_t *) curr_proc->pde_paddr);

}

void syscall_write(FullTrapFrame *tf, uintptr_t pc) {
    debug_printf("syscall writing...\n");

    struct Proc * curr_proc = get_curr();
    uint32_t fd = SYSCALL_ARG0(tf);
    uint32_t src_vaddr = SYSCALL_ARG1(tf);
    uint32_t size = SYSCALL_ARG2(tf);

    switch_to_kernel_tables();
    int *src = (int *) get_paddr_for((paddr_t*)curr_proc->pde_paddr, src_vaddr);
    struct File *f = curr_proc->files[fd];

    if (f == NULL){
        SET_SYSCALL_RET0(tf, -1);
        switch_page_table((uint32_t *) curr_proc->pde_paddr);
        return;
    }

    int success = fd_write(f, src, size);
    SET_SYSCALL_RET0(tf, success);
    switch_page_table((uint32_t *) curr_proc->pde_paddr);
}

void syscall_read(FullTrapFrame *tf, uintptr_t pc) {
    debug_printf("syscall FD read...\n");
    struct Proc * curr_proc = get_curr();
    uint32_t fd = SYSCALL_ARG0(tf);
    uint32_t dst_vaddr = SYSCALL_ARG1(tf);
    uint32_t size = SYSCALL_ARG2(tf);
    switch_to_kernel_tables();

    int *dst = (int *) get_paddr_for((paddr_t*)curr_proc->pde_paddr, dst_vaddr);
    struct File *f = curr_proc->files[fd];

    if (f == NULL){
        SET_SYSCALL_RET0(tf, -1);
        switch_page_table((uint32_t *) curr_proc->pde_paddr);
        return;
    }

    int success = fd_read(f, dst, size);
    SET_SYSCALL_RET0(tf, success);
    switch_page_table((uint32_t *) curr_proc->pde_paddr);
}

void syscall_dup(FullTrapFrame *tf, uintptr_t pc) {
    debug_printf("syscall dup...\n");
    int prev_fd = SYSCALL_ARG0(tf);
    struct Proc *curr_proc = get_curr();

    if (prev_fd < 0 || prev_fd >= MAX_FILES || curr_proc->files[prev_fd] == NULL) {
        SET_SYSCALL_RET0(tf, -1);
        return;
    }

    int dup_fd = get_file_descriptor(curr_proc);
    curr_proc->files[dup_fd] = curr_proc->files[prev_fd];

    fd_retain(curr_proc->files[dup_fd]);

    SET_SYSCALL_RET0(tf, dup_fd);
}

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


void syscall_alive(FullTrapFrame *tf, uintptr_t pc){
    uint32_t pid = SYSCALL_ARG0(tf);
    struct Proc *proc = get_proc(pid);
    uint32_t is_alive = proc->status == PROC_RUNNABLE || proc->status == PROC_RUNNING; 
    SET_SYSCALL_RET0(tf, is_alive);
}


void syscall_virtual_copy(FullTrapFrame *tf, uintptr_t pc){

    //HAY QUE AGREGAR UN PARAMETRO MAS EN LA LLAMADA A SYSCALL

    /*uint32_t src_vaddr = SYSCALL_ARG0(tf);
    uint32_t dst_vaddr = SYSCALL_ARG1(tf);
    uint32_t len = SYSCALL_ARG2(tf);
    int32_t result;

    switch_to_kernel_tables();

    uint32_t src_paddr =
    uint32_t src_paddr =

    switch_page_table((uint32_t *) get_curr()->pde_paddr);*/

    SET_SYSCALL_RET0(tf, 0);
}

void init_syscalls_ipc(void){
    register_syscall(SYS_TRY_SEND_CONTENT, syscall_try_send_content);
    register_syscall(SYS_TRY_RECV_CONTENT, syscall_try_recv_content);
    register_syscall(SYS_RECV_CONTENT, syscall_recv_content);
}

void init_syscalls_proc(void) {
    register_syscall(SYS_EXEC, syscall_exec);
    register_syscall(SYS_EXIT, syscall_exit);
    register_syscall(SYS_WAIT, syscall_wait);
    register_syscall(SYS_YIELD, syscall_yield);
    register_syscall(SYS_GETPID, syscall_getpid);
    register_syscall(SYS_COORDPID, syscall_get_coord_pid);
    register_syscall(SYS_ALIVE, syscall_alive);
    register_syscall(SYS_VIRTUAL_COPY, syscall_virtual_copy);
}

