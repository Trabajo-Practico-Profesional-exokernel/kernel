#ifndef IPC_H
#define IPC_H

#include "types.h"
#include "ipc_msgs.h"
#include "inc/operations.h"

#define MAX_BUFFER_IPC_SIZE 1024

int32_t app_try_recv_msg(Msg *msg);

int32_t app_receive_parameter(uint32_t operation);

int32_t app_receive_content(uint32_t operation, char *buffer, int len);

int32_t server_send_parameter_to_app(int32_t pid, uint32_t operation, int32_t arg);

int32_t server_send_content_to_app(int32_t pid, uint32_t operation, char *buffer, int len, int32_t value);

int32_t app_send_msg_to_server(int32_t arg_1, int32_t arg_2, int32_t arg_3, int32_t arg_4, int32_t arg_5, int32_t arg_6);

int32_t recv_msg(Msg *msg);
int32_t send_msg(int32_t recv_pid, int32_t arg_1, int32_t arg_2, int32_t arg_3, int32_t arg_4, int32_t arg_5, int32_t arg_6);
int32_t server_get_real_fd(int32_t fd, int32_t app_pid, int32_t type_server);

#endif
