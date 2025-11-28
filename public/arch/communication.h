#ifndef PUBLIC_ARCH_COMMUNICATION_H
#define PUBLIC_ARCH_COMMUNICATION_H

#include "inc/types.h"

#define MSG_SIZE_MAX 64 
#define QUEUE_CAPACITY 64

struct Message {
    uint8_t sender_pid;
    uint8_t receiver_pid;
    uint8_t content[MSG_SIZE_MAX];
};

struct MessageMetadata {
    bool ready_to_read;
    uint8_t content_size;
    uint8_t read_index;
};

struct IPC_Message{
    struct Message message;
    struct MessageMetadata message_metadata;
};

struct ProcMessageQueue
{
    struct IPC_Message msg_pend_message_queue[QUEUE_CAPACITY];
    struct IPC_Message msg_ready_message_queue[QUEUE_CAPACITY];
    uint8_t msg_pend_message_count;
    uint8_t msg_ready_message_count;
};

struct MessageQueue {
    uint8_t msg_count;
    struct Message* next;
};

struct MessageIPCQueue {
    uint8_t msg_count;
    struct IPC_Message* next;
};

struct CharMessage {
    uint8_t sender_pid;
    uint8_t char_content;
};

struct ProcessMessage {
    uint8_t sender_pid;
    uint8_t content[MSG_SIZE_MAX];
    uint8_t actual_content_size;
    uint8_t actual_index;
    bool ready_to_read;
    bool reserved;
};

#endif