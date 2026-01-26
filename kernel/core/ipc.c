#include "ipc.h"
#include "string.h"
#include "constants.h"
#include "types.h"
#include "arch/mem.h"
#include "stdio.h"
#include "sched.h"
#include "stdlib.h"
#include "console/debug.h"
#include "arch/proc.h"

struct IpcManager ipc_manager;

void init_ipc_manager(){
    memset(&ipc_manager, 0, sizeof(struct IpcManager));
}

int32_t buffer_read_content(struct Buffer *buf, uint8_t *dst, uint8_t len){
    if (len > buf->index){
        return ERROR;
    }

    memcpy(dst, buf->content, len);

    uint8_t remaining = buf->index - len;
    if (remaining > 0){
        memmove(buf->content, &buf->content[len], remaining);
    }

    buf->index = remaining;
    return SUCCESS;
}

int32_t buffer_write_content(struct Buffer *buf, const uint8_t *src, uint8_t len){
    if ((uint16_t)buf->index + len > MAX_IPC_BUFFER_BYTES){
        return ERROR;
    }

    memcpy(&buf->content[buf->index], src, len);
    buf->index += len;
    return SUCCESS;
}

uint32_t send_content(uint32_t sender_proc_id, uint32_t receiver_proc_id, uint32_t content_virt_addr, uint32_t len){
    
    struct Proc *sender_proc = get_curr();

    struct Proc *receiver_proc = get_proc(receiver_proc_id);

    if (!receiver_proc) return ERROR;

    uint32_t content_phys_addr = get_paddr_for((uint32_t*)sender_proc->pde_paddr, content_virt_addr);

    struct Buffer *buf = &ipc_manager.msg_buffer[receiver_proc_id];
    
    uint32_t result = buffer_write_content(buf, (uint8_t*)content_phys_addr, len);
    
    if (result == ERROR){
        return ERROR;
    }

    receiver_proc->status = PROC_RUNNABLE;
    return SUCCESS;
}

uint32_t recv_content(uint32_t receiver_proc_id, uint32_t content_virt_addr, uint32_t len){
    
    struct Proc *current_proc = get_curr(); 

    uint32_t content_phys_addr = get_paddr_for((uint32_t*)current_proc->pde_paddr, content_virt_addr);

    struct Buffer *buf = &ipc_manager.msg_buffer[receiver_proc_id];

    uint32_t result = buffer_read_content(buf, (uint8_t*)content_phys_addr, len);
    
    return result;
}
