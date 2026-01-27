#ifndef IPC_H
#define IPC_H

#include "types.h"
#include "ipc_msgs.h"
#include "inc/operations.h" // Aquí está definido el enum Server

int32_t send_msg_to_service(int32_t type_msg, 
    int32_t arg_1, int32_t arg_2, int32_t arg_3, 
    void* content, int32_t len_content);

int32_t send_msg_to_app(int32_t app_id, int32_t type_msg, int32_t arg_1, int32_t arg_2);

int32_t receive_msg(ServiceMsgUnion *buffer);

#endif
