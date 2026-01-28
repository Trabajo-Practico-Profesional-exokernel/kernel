#include "ipc.h"
#include "syscalls.h" 
#include "string.h"
#include "lib.h" 
#include "constants.h"
#include "inc/operations.h"
#include "ipc_msgs.h"

uint8_t ipc_buffer[MAX_BUFFER_IPC_SIZE] = {0};

int32_t send_msg(int32_t recv_pid, int32_t arg_1, int32_t arg_2, int32_t arg_3, int32_t arg_4, int32_t arg_5, int32_t arg_6){
    int32_t sender_pid = getpid();
    Msg msg = {sender_pid, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6};
    return try_send_content(recv_pid, (char*)&msg, sizeof(Msg));
}

int32_t recv_msg(Msg *msg) {
    if (!msg) {
        return ERROR;
    }
    return recv_content((char*)msg, sizeof(Msg));
}

int32_t send_ack(int32_t sender_pid, int32_t operation){
    return send_msg(sender_pid, operation, 0, 0, 0, 0, 0);
}

int32_t recv_ack(int32_t recv_pid, int32_t operation) {
    Msg msg;
    int32_t res = recv_content((char*)&msg, sizeof(Msg));
    if (res == ERROR) {
        return ERROR;
    }

    return (msg.sender_pid == recv_pid && msg.arg_1 == operation);
}


int32_t app_receive_parameter(uint32_t operation) {
    Msg msg;

    int32_t res = recv_msg(&msg);
    if (res == ERROR) {
        return ERROR;
    }

    if (msg.arg_1 != operation){
        return ERROR;
    }

    return msg.arg_2;
}

int32_t app_receive_content(uint32_t operation, char *buffer, int len){
    Msg msg;
    int32_t res = recv_msg(&msg);

    if (res == ERROR) {
        return ERROR;
    }

    if (msg.arg_1 != operation || msg.arg_2 == ERROR){
        return ERROR;
    }

    //FALTA VERIFICAR LA LONGITUD DEL BUFFER VIRTUAL
    res = virtual_copy(msg.sender_pid, msg.arg_3, (uint32_t)buffer, len);
    if (res == ERROR){
        //ver si conviene mas devolver un NACK
        send_ack(msg.sender_pid, operation);
        return ERROR;
    }

    send_ack(msg.sender_pid, operation);
    return SUCCESS;
}

int32_t server_send_parameter_to_app(int32_t pid, uint32_t operation, int32_t arg) {
    return send_msg(pid, operation, arg, 0, 0, 0, 0);
}

int32_t server_send_content_to_app(int32_t pid, uint32_t operation, char *buffer, int len){

    int32_t res;
    memcpy(&ipc_buffer, buffer, len);
    
    res = send_msg(pid, operation, SUCCESS, (int)buffer, len, 0, 0);
    if (res == ERROR){
        return ERROR;
    } 

    res = recv_ack(pid, operation);
    memset(&ipc_buffer, 0, MAX_BUFFER_IPC_SIZE);
    return res;
}

int32_t get_server_pid(int32_t arg_1, int32_t arg_2, int32_t arg_3,
    int32_t arg_4, int32_t arg_5, int32_t arg_6){
    int32_t res;
    int32_t coordinator_pid = get_coord_pid();                  
    res =  send_msg(coordinator_pid, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
    if (res == ERROR){
        return ERROR;
    }

    Msg coordinator_response;
    res = recv_msg(&coordinator_response);
    if (res = ERROR){
        return ERROR;
    }

    res = coordinator_response.arg_1;

    if (res = ERROR){
        return ERROR;
    }

    return coordinator_response.arg_2;
}

int32_t app_send_msg_to_server(int32_t arg_1, int32_t arg_2, int32_t arg_3,
                                int32_t arg_4, int32_t arg_5, int32_t arg_6){

    int32_t server_pid = get_server_pid(arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);

    if (server_pid < 0){
        return ERROR;
    }

    return send_msg(server_pid, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
}
