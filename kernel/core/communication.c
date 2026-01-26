#include "arch/communication.h"
#include "types.h"
#include "arch/mem.h"
#include "stdio.h"
#include "arch/proc.h"
#include "sched.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "console/debug.h"

int copyin_msg(struct Proc *p, char *dst, vaddr_t src_va, int max_len) {

    paddr_t src_pa = get_paddr_for((uint32_t*)p->pde_paddr, src_va);
    int i;
    for(i = 0; i < max_len; i++){
        paddr_t pa_byte = src_pa + i;

        char c = *(char*)pa_byte;
        dst[i] = c;
        if(c == '\0') return i;
    }
    return -1;
}

int copyout_msg(struct Proc *p, vaddr_t dst_va, void *src, int len) {

    paddr_t dst_pa = get_paddr_for((uint32_t*)p->pde_paddr, dst_va);
    
    char *k_src = (char *)src;
    int i;

    for(i = 0; i < len; i++){
        paddr_t pa_byte = dst_pa + i;
        
        *(char*)pa_byte = k_src[i];
    }

    return 0;
}

int insert_msg(struct Proc *receiver_proc, struct Message msg){
    if (receiver_proc->msgs_queue.len_queue < QUEUE_CAPACITY){
        
        int index = receiver_proc->msgs_queue.len_queue;
        receiver_proc->msgs_queue.msg_queue[index] = msg;
        
        receiver_proc->msgs_queue.len_queue++;

        return 1;
    }
    return 0;
}

struct Message extract_msg(struct Proc *receiver_proc){
    
    if (receiver_proc->msgs_queue.len_queue == 0) {
        struct Message empty_msg = {0};
        return empty_msg;
    }

    struct Message msg = receiver_proc->msgs_queue.msg_queue[0];

    for (int i = 0; i < receiver_proc->msgs_queue.len_queue - 1; i++) {
        receiver_proc->msgs_queue.msg_queue[i] = receiver_proc->msgs_queue.msg_queue[i+1];
    }

    receiver_proc->msgs_queue.len_queue--;

    return msg;
}

int send_msg(struct Proc * sender_proc, int receiver_proc_pid, uint32_t msg_addr, int type_msg){
    struct Message msg;
    int content_size = copyin_msg(sender_proc, msg.content, msg_addr, MSG_SIZE_MAX);

    if (content_size < 0) {
        return 0;
    }

    msg.sender_pid = sender_proc->pid;
    msg.content_size = content_size;
    msg.type = type_msg;

    struct Proc* receiver_proc = get_proc(receiver_proc_pid);
    
    int success = insert_msg(receiver_proc, msg);
    
    if (success) {
        receiver_proc->status = PROC_RUNNABLE;
        return 1;
    } else {
        return 0;
    }
}

int recv_msg(struct Proc* receiver_proc, uint32_t msg_vaddr) {
    struct Message msg = extract_msg(receiver_proc);

    if (msg.type == 0) {
        return -1;
    }

    copyout_msg(receiver_proc, msg_vaddr, &msg, sizeof(struct Message));
    return 0;
}
