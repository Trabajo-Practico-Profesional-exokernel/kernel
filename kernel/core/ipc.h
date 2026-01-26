#ifndef IPC_H
#define IPC_H

#include "arch/proc.h"
#include "types.h"

#define MAX_IPC_BUFFER_BYTES 512

struct Buffer {
    uint8_t content[MAX_IPC_BUFFER_BYTES];
    uint8_t index;
};

struct IpcManager {
    struct Buffer msg_buffer[PROCS_MAX];
};

void init_ipc_manager();

uint32_t send_content(uint32_t sender_proc_id, uint32_t receiver_proc_id, uint32_t content_addr, uint32_t len_content);

uint32_t recv_content(uint32_t receiver_proc_id, uint32_t content_addr, uint32_t len_content);

#endif