#include "ipc.h"
#include "proc.h"
#include "std/string.h"

#define LAST_MSG_BYTE (uint8_t)'\0'

void init_proc_queue(struct Proc *proc){
    proc->proc_msg_queue.msg_pend_message_count = 0;
    proc->proc_msg_queue.msg_ready_message_count = 0;
    memset((void*)proc->proc_msg_queue.msg_pend_message_queue, 0, MSG_SIZE_MAX * sizeof(struct IPC_Message));
    memset((void*)proc->proc_msg_queue.msg_ready_message_queue, 0, MSG_SIZE_MAX * sizeof(struct IPC_Message));
}

bool msg_available(struct Proc * proc){
    return proc->proc_msg_queue.msg_ready_message_count >0;
}

bool msg_queue_full(struct Proc * proc){
    return proc->proc_msg_queue.msg_pend_message_count >=MSG_SIZE_MAX 
        || proc->proc_msg_queue.msg_ready_message_count >=MSG_SIZE_MAX;
}

bool recv_msg_byte(struct Proc * proc, uint8_t *byte){
    if (msg_available(proc)){
        struct IPC_Message *msg = &proc->proc_msg_queue.msg_ready_message_queue[0];
        uint8_t index = msg->message_metadata.read_index;
        *byte = msg->message.content[index];
        msg->message_metadata.read_index++;

        if (msg->message_metadata.read_index == msg->message_metadata.content_size){
            msg->message_metadata.ready_to_read = false;

            for (int j = 0; j < proc->proc_msg_queue.msg_ready_message_count - 1; j++) {
                proc->proc_msg_queue.msg_ready_message_queue[j] = proc->proc_msg_queue.msg_ready_message_queue[j+1];
            }
            proc->proc_msg_queue.msg_ready_message_count --;
        }
        return true;
    }
    return false;
}

bool send_msg_byte(struct Proc * proc, uint8_t sender_pid, uint8_t byte){

    if (msg_queue_full(proc)){
        return false;
    }

    for (int i=0; i<proc->proc_msg_queue.msg_pend_message_count; i++){
        struct IPC_Message *msg = &proc->proc_msg_queue.msg_pend_message_queue[i];

        if (msg->message.sender_pid == sender_pid){

            int index = msg->message_metadata.content_size;
            msg->message.content[index] = byte;
            msg->message_metadata.content_size ++;

            if (byte == LAST_MSG_BYTE){

                int *ready_msg_index =  &proc->proc_msg_queue.msg_ready_message_count;
                proc->proc_msg_queue.msg_ready_message_queue[*ready_msg_index] = *msg;
                (*ready_msg_index) ++;
                
                for (int j = i; j < proc->proc_msg_queue.msg_pend_message_count - 1; j++) {
                    proc->proc_msg_queue.msg_pend_message_queue[j] = proc->proc_msg_queue.msg_pend_message_queue[j+1];
                }

                proc->proc_msg_queue.msg_pend_message_count --;

                if (proc->status == PROC_NOT_RUNNABLE){
                    proc->status = PROC_RUNNABLE;
                }
            }
            return true;
        }
    }
    if (byte == LAST_MSG_BYTE){
        return true;
    }

    struct IPC_Message new_msg = create_msg(sender_pid, byte, proc->pid);

    int *index_pend_queue = &proc->proc_msg_queue.msg_pend_message_count;
    proc->proc_msg_queue.msg_pend_message_queue[*index_pend_queue] = new_msg;;

    (*index_pend_queue)++;

    if (proc->status == PROC_NOT_RUNNABLE){
        proc->status = PROC_RUNNABLE;
    }

    return true;
}

struct IPC_Message create_msg(uint8_t sender_pid, uint8_t byte, uint8_t receiver_pid){
    struct IPC_Message new_msg;

    new_msg.message.content[0] = byte;
    new_msg.message.sender_pid = sender_pid;
    new_msg.message.receiver_pid = receiver_pid;

    new_msg.message_metadata.content_size = 1;
    new_msg.message_metadata.read_index = 0;

    return new_msg;
}
