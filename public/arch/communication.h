#ifndef PUBLIC_ARCH_COMMUNICATION_H
#define PUBLIC_ARCH_COMMUNICATION_H

#include "inc/types.h"

#define MSG_SIZE_MAX 64 

struct CharMessage {
    uint32_t sender_pid;
    char char_content;
};

struct ProcessMessage {
    uint32_t sender_pid;
    uint8_t content[MSG_SIZE_MAX];
    size_t actual_content_size;
    bool ready_to_read;
    bool reserved;
};

struct Message {
    uint32_t sender_pid;
    uint32_t receiver_pid;
    char content[MSG_SIZE_MAX];
    size_t size;
};

#endif 