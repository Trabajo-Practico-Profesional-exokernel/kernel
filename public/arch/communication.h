#ifndef PUBLIC_ARCH_COMMUNICATION_H
#define PUBLIC_ARCH_COMMUNICATION_H

#include "inc/types.h"
#include "mem.h"
#include "arch_inc/trapframe.h"
#define MSG_SIZE_MAX 64 
#define QUEUE_CAPACITY 64

#define FS_TYPE_OPEN    0
#define FS_TYPE_CLOSE   1
#define FS_TYPE_FSTAT   2
#define FS_TYPE_MKDIR   3
#define FS_TYPE_UNLINK  4
#define FS_TYPE_LINK    5
#define FS_TYPE_MKNOD   6
#define FS_TYPE_CHDIR   7

struct Proc;

struct Message {
    uint8_t sender_pid;
    uint8_t content_size;
    uint8_t content[MSG_SIZE_MAX];
    uint8_t type;
};

struct MessageQueue {
    struct Message msg_queue[QUEUE_CAPACITY];
    uint8_t len_queue;
};

int copyin_msg(struct Proc *p, char *dst, vaddr_t src_va, int max_len);
int copyout_msg(struct Proc *p, vaddr_t dst_va, void *src, int len);

int insert_msg(struct Proc *receiver_proc, struct Message msg);
struct Message extract_msg(struct Proc *receiver_proc);
int send_msg(struct Proc * sender_proc, int receiver_proc_pid, uint32_t msg_addr, int len_msg, int msg_type);
int recv_msg(FullTrapFrame *tf, uintptr_t pc, bool blocking, uint32_t msg_addr);
#endif