#include "arch/communication.h"
#include "inc/types.h"
#include "arch/mem.h"
#include "std/printf.h"
#include "arch/proc.h"

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