#ifndef PUBLIC_ARCH_COMMUNICATION_H
#define PUBLIC_ARCH_COMMUNICATION_H

#include "inc/types.h"
#include "mem.h"

#define MSG_SIZE_MAX 64 
#define QUEUE_CAPACITY 64

struct Proc;

struct Message {
    uint8_t sender_pid;
    uint8_t content_size;
    uint8_t content[MSG_SIZE_MAX];
};

struct MessageQueue {
    struct Message msg_queue[QUEUE_CAPACITY];
    uint8_t len_queue;
};

int copyin_msg(struct Proc *p, char *dst, vaddr_t src_va, int max_len);
int copyout_msg(struct Proc *p, vaddr_t dst_va, void *src, int len);

int insert_msg(struct Proc *receiver_proc, struct Message msg);
struct Message extract_msg(struct Proc *receiver_proc);

#endif