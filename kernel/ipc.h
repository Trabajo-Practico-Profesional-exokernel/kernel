#include "inc/types.h"
#include "proc.h"
#define MSG_SIZE_MAX 64
#define QUEUE_CAPACITY 64

void init_proc_queue(struct Proc *proc);

bool recv_msg_byte(struct Proc * proc, uint8_t *byte);
bool send_msg_byte(struct Proc * proc, uint8_t sender_pid, uint8_t byte);
bool msg_available(struct Proc * proc);
bool msg_queue_full(struct Proc * proc);

struct IPC_Message create_msg(uint8_t sender_pid, uint8_t byte, uint8_t pid);

struct Message get_msg(struct Proc * proc);
