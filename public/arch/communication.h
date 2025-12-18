#ifndef PUBLIC_ARCH_COMMUNICATION_H
#define PUBLIC_ARCH_COMMUNICATION_H

#include "inc/types.h"

#define MSG_SIZE_MAX 64 
#define QUEUE_CAPACITY 64

struct Message {
    uint8_t sender_pid;
    uint8_t content_size;
    uint8_t content[MSG_SIZE_MAX];
};

struct MessageNode {
    struct Message message;
    struct Message* next;
};

// struct ProcMessageQueue
// {
//     struct MessageNode* messa;

// };

// [] / 4096

// [ msg 1 = 5] / 4091
// [msg 1 =5, msg2= 10] 4081
// [ free = 5 , msg2= 10] /4081
// [ free = 5 , msg2= 10, msg3 = 10] /4071
// msg3 = 5
// uintptr_t msgs_queue= alloc_page(1) 
// 




#endif