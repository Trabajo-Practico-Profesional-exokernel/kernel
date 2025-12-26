#include "arch/communication.h"
#include "inc/types.h"
#include "arch/mem.h"
#include "std/printf.h"
#include "arch/proc.h"
#include "sched.h"

int copyin_msg(struct Proc *p, char *dst, vaddr_t src_va, int max_len) {
    int i;
    for(i = 0; i < max_len; i++){
        vaddr_t va_byte = src_va + i;
        paddr_t pa_byte = get_paddr_for((paddr_t*)p->pde_paddr, va_byte);
        if(pa_byte == 0) return -1;

        char c = *(char*)pa_byte;
        dst[i] = c;
        if(c == '\0') return i;
    }
    return -1;
}

int copyout_msg(struct Proc *p, vaddr_t dst_va, void *src, int len) {
    char *k_src = (char *)src;
    int i;

    for(i = 0; i < len; i++){
        vaddr_t va_byte = dst_va + i;

        paddr_t pa_byte = get_paddr_for((uint32_t*)p->pde_paddr, va_byte);
        
        if(pa_byte == 0) return -1; 

        *(char*)pa_byte = k_src[i];
    }

    return 0;
}


int insert_msg(struct Proc *receiver_proc, struct Message msg){
    int *queue_index = &receiver_proc->msgs_queue.len_queue;

    if (*queue_index < QUEUE_CAPACITY){

        receiver_proc->msgs_queue.msg_queue[*queue_index] = msg;
        (*queue_index)++;

        return 1;
    }
    return 0;
}

struct Message extract_msg(struct Proc *receiver_proc){

    int *queue_index = &receiver_proc->msgs_queue.len_queue;
    
    if (*queue_index <= 0) {
        struct Message empty_msg = {0};
        return empty_msg;
    }

    struct Message msg = receiver_proc->msgs_queue.msg_queue[*queue_index - 1];
    (*queue_index)--;

    return msg;
}

int send_msg(struct Proc * sender_proc, int receiver_proc_pid, uint32_t msg_addr, int len_msg){
    struct Message msg;
    msg.sender_pid = sender_proc->pid;
    msg.content_size = len_msg;
    copyin_msg(sender_proc, msg.content, msg_addr, len_msg);

    printf("AFTER COPYIN MSG...\n");
    struct Proc* receiver_proc = get_proc(receiver_proc_pid);
    
    int success = insert_msg(receiver_proc, msg);
    if (success) {

        if (receiver_proc->status == PROC_NOT_RUNNABLE){
            receiver_proc->status = PROC_RUNNABLE;
        }
        return 1;
    } else {
        return 0;
    }
}

int recv_msg(FullTrapFrame *tf, uintptr_t pc, bool blocking) {
    struct Proc *receiver_proc = get_curr();
    uint32_t msg_addr = SYSCALL_ARG0(tf);

    struct Message msg = extract_msg(receiver_proc);

    if (msg.content_size == 0) {
        if (blocking) {
            receiver_proc->status = PROC_NOT_RUNNABLE;
            save_curr_proc_state(tf, pc);
            sched_yield();
        }
        return 0;
    }

    copyout_msg(receiver_proc, msg_addr, msg.content, msg.content_size);
    return 1;
}