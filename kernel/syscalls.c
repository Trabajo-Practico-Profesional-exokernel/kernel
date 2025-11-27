#include "inc/common.h"
#include "inc/syscalls.h"
#include "arch/trap_handling.h"
#include "arch/logging.h"
#include "std/string.h"
#include "arch/communication.h"
#include "arch_inc/trap_constants.h"
#include "arch/stdio.h"


// Sched exec , wait and so on...
#include "sched.h"
#include "proc.h"
#include "arch/mem.h" // needed for switch to kernel page tables

#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  
#ifdef IS_RISC
// meta/gen/apps_meta.c defines this...
extern struct AppBinaryInfo _binary_apps[];
#else
struct AppBinaryInfo _binary_apps[10];
#endif

void syscall_exec(FullTrapFrame *tf, uintptr_t pc) {

    int prog_ind = SYSCALL_ARG0(tf);

    if (prog_ind < 0 || prog_ind>= APP_COUNT){
        printf("Invalid exec call ind %d \n", prog_ind);
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)        
        return;
    }
    printf("Should run program at ind %d \n", prog_ind);
    
    struct Proc* proc= get_first_free_proc();
    // It cannot but NULL it throws panic for now but check it anyway for the future!
    if (proc == NULL){
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)
        return;
    }

    printf("Should run free proc %p binary: %p \n", proc, &_binary_apps[prog_ind]);

    #ifdef IS_RISC
    switch_to_kernel_tables();
    load_create_process_user(proc, &_binary_apps[prog_ind]);
    
    // Do switch to new proc? ... no?
    SET_SYSCALL_RET0(tf, proc->pid)

    // Switch back to page table of user!
    switch_page_table((uint32_t *)(get_curr()->pde_paddr));
    #else

    load_create_process_user(proc, &_binary_apps[prog_ind]);
    // Now do switch? or not? naaa If you want you could wait for it! after ret.
    SET_SYSCALL_RET0(tf, proc->pid)
    #endif

}


void syscall_exit(FullTrapFrame *tf, uintptr_t pc){
    int exit_code = SYSCALL_ARG0(tf);
    struct Proc * exited_proc = get_curr();
    printf("Process %u exited with code %d\n", exited_proc->pid, exit_code);

    #ifdef IS_RISC
    switch_to_kernel_tables();
    #endif
    
    // Delete! .. from proc.c
    free_process(exited_proc);

    // TO DO! Notify for processes waiting?
    sched_yield();
}

void syscall_yield(FullTrapFrame *tf, uintptr_t pc){
    save_curr_proc_state(tf, pc);
    sched_yield();
}


void syscall_wait(FullTrapFrame *tf, uintptr_t pc){
    int waited_proc = SYSCALL_ARG0(tf);
    struct Proc * waiting_proc = get_curr();
    printf("Process %d should wait blocked for %d exit!(For now just yield!)\n",waiting_proc->pid,  waited_proc);
    
    SET_SYSCALL_RET0(tf, 0) // On tf saved... save hardcoded ret code for waited proc

    save_curr_proc_state(tf, pc);
    sched_yield();    
}




void syscall_putchar(FullTrapFrame *tf, uintptr_t pc) {
    putchar(SYSCALL_ARG0(tf));
}



void syscall_getchar(FullTrapFrame *tf, uintptr_t pc) {
    while (1) {
        long ch = getchar();
        if (ch >= 0) {
            SET_SYSCALL_RET0(tf, ch); // change sys ret vl
            break;
        }

        // save_curr_proc_state(tf, pc);
        // get_curr()->status = PROC_NOT_RUNNABLE;
        // sched_yield();
    }            
}


void send_msg_to_process(struct CharMessage new_char_message, struct Proc *proc_receiver){

    //TODO FIX: se suman mensjaes por cada caracter agregado

    for (int i=0; i<proc_receiver->msg_count; i++){
        struct ProcessMessage *actual_proc_receiver_msg = &proc_receiver->msg_queue[i];
        if (actual_proc_receiver_msg->sender_pid == new_char_message.sender_pid && !actual_proc_receiver_msg->ready_to_read){

            int index = actual_proc_receiver_msg->actual_content_size;
            actual_proc_receiver_msg->content[index] = new_char_message.char_content;
            actual_proc_receiver_msg->actual_content_size ++;
            if (new_char_message.char_content == '\0'){
                actual_proc_receiver_msg->ready_to_read = true;

                if (proc_receiver->status == PROC_NOT_RUNNABLE) {
                    proc_receiver->status = PROC_RUNNABLE;
                }

            }
            return;
        }
    }

    if (new_char_message.char_content == '\0') {
        return;
    }

    struct ProcessMessage new_msg;
    new_msg.sender_pid = new_char_message.sender_pid;
    new_msg.content[0] = new_char_message.char_content;
    new_msg.actual_content_size = 1;
    //cambiar ready to read a true
    new_msg.ready_to_read = false;
    new_msg.reserved = true;

    proc_receiver->msg_queue[proc_receiver->msg_count] = new_msg;
    proc_receiver->msg_count++;

    if (proc_receiver->status == PROC_NOT_RUNNABLE) {
        printf("[KERNEL] Despertando proceso %d (nuevo mensaje)\n", proc_receiver->pid);
        proc_receiver->status = PROC_RUNNABLE;
    }
}

void syscall_send_char(FullTrapFrame *tf, uintptr_t pc){

    uint32_t receiver_pid = SYSCALL_ARG0(tf);
    char char_msg = SYSCALL_ARG1(tf);

    struct Proc *proc_receiver = get_proc_by_pid(receiver_pid);
    struct Proc *proc_sender = get_curr();

    if (proc_receiver == NULL || proc_receiver->status == PROC_FREE) {
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE);
        return;
    }

    struct CharMessage new_char_message;
    new_char_message.char_content = char_msg;
    new_char_message.sender_pid = proc_sender->pid;

    if (proc_receiver == NULL || proc_receiver->status == PROC_FREE) {
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE);
        return;
    }

    send_msg_to_process(new_char_message, proc_receiver);

    SET_SYSCALL_RET0(tf, 0);
}

void syscall_recv_msg(FullTrapFrame *tf, uintptr_t pc) {

    printf("[KERNEL] buscando caracter!\n");

    struct Proc *current_proc = get_curr();

    if (current_proc->msg_count == 0) {
        save_curr_proc_state(tf, pc);
        current_proc->status= PROC_NOT_RUNNABLE;
        sched_yield();
        return;
    }
    current_proc = get_curr();
    printf("[KERNEL] antes del for!\n");
    for (int i=0; i<current_proc->msg_count; i++){
        if (current_proc->msg_queue[i].ready_to_read){


            //char *user_buffer = (char *)SYSCALL_ARG1(tf);
    
            struct ProcessMessage *msg = &current_proc->msg_queue[i];
            SET_SYSCALL_RET0(tf, (int)msg->content[0]);
            //provisorio
            msg->ready_to_read = false;
            //*user_buffer = msg.content[0];

            for (int j = i; j < current_proc->msg_count - 1; j++) {
                current_proc->msg_queue[j] = current_proc->msg_queue[j + 1];
            }

            current_proc->msg_count--;

            printf("[KERNEL] caracter! '%c' \n", msg->content[0]);

            
            return;
        }   
    }

    save_curr_proc_state(tf, pc);
    current_proc->status= PROC_NOT_RUNNABLE;
    sched_yield();
    return;
}


void syscall_send_msg(FullTrapFrame *tf, uintptr_t pc) {
}


#define MAX_SYSCALLS 32


syscall_handler_t syscall_table[MAX_SYSCALLS] = {
    [SYS_PUTCHAR] = syscall_putchar,
    [SYS_GETCHAR] = syscall_getchar,
    [SYS_EXEC] = syscall_exec,

    [SYS_EXIT] = syscall_exit,
    [SYS_WAIT] = syscall_wait,
    [SYS_YIELD] = syscall_yield,
    [SYS_SEND_MSG] = syscall_send_msg,
    [SYS_RECV_MSG] = syscall_recv_msg,
    [SYS_SENDCHAR] = syscall_send_char,
    // ... other handlers
};

uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc) {
    unsigned sysno = SYSCALL_SYSNO(tf);

    if (sysno >= MAX_SYSCALLS || syscall_table[sysno] == NULL) {
        printf("unexpected syscall a3=%x max sysno: %x at pc: %x\n", sysno, MAX_SYSCALLS, pc);
        printTrapFull(tf);
    } else {
        syscall_table[sysno](tf, pc);
    }

    return pc + 4; // skip ecall
}