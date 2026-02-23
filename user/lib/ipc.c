#include "ipc.h"
#include "syscalls.h" 
#include "string.h"
#include "lib.h" 
#include "constants.h"
#include "inc/operations.h"
#include "ipc_msgs.h"

uint8_t ipc_buffer[MAX_BUFFER_IPC_SIZE] = {0};

#define MAX_APP_ATTEMPTS 50


int32_t send_msg(int32_t recv_pid, int32_t arg_1, int32_t arg_2, int32_t arg_3, int32_t arg_4, int32_t arg_5, int32_t arg_6){
    int32_t sender_pid = getpid();
    printf("[IPC] send_msg: Sender [%d] -> Recv [%d] | OP: %d | args: %d, %d, %d\n", sender_pid, recv_pid, arg_1, arg_2, arg_3, arg_4);
    Msg msg = {sender_pid, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6};
    return try_send_content(recv_pid, (char*)&msg, sizeof(Msg));
}

int32_t recv_msg(Msg *msg) {
    if (!msg) {
        return ERROR;
    }
    return recv_content((char*)msg, sizeof(Msg));
}

int32_t app_try_recv_msg(Msg *msg) {
    if (!msg) return ERROR;

    for (int i = 0; i<MAX_APP_ATTEMPTS; i++){
        if (try_recv_content((char*)msg, sizeof(Msg))==SUCCESS){
            printf("[IPC] recv_msg: PID [%d] received OP %d from PID [%d]\n", getpid(), msg->arg_1, msg->sender_pid);
            return SUCCESS;
        }
        sys_yield();
    }
    printf("[IPC] recv_msg: TIMEOUT PID [%d]\n", getpid());
    return ERROR;
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

    int32_t res = app_try_recv_msg(&msg);
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
    int32_t res = app_try_recv_msg(&msg);

    if (res == ERROR) {
        return ERROR;
    }

    if (msg.arg_1 != operation || msg.arg_2 == ERROR){
        return ERROR;
    }

    int copy_len = (msg.arg_4 < len) ? msg.arg_4 : len;
    
    if (copy_len > 0) {
        res = virtual_copy(msg.sender_pid, msg.arg_3, (uint32_t)buffer, copy_len);
        if (res == ERROR) {
            return ERROR;
        }
        
        send_ack(msg.sender_pid, operation);
    }

    return msg.arg_2;
}

int32_t server_send_parameter_to_app(int32_t pid, uint32_t operation, int32_t arg) {
    return send_msg(pid, operation, arg, 0, 0, 0, 0);
}

int32_t server_send_content_to_app(int32_t pid, uint32_t operation, char *buffer, int len, int32_t value){

    int32_t res;
    memcpy(&ipc_buffer, buffer, len);
    res = send_msg(pid, operation, value, (int)&ipc_buffer, len, 0, 0);
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

    if (res == ERROR){
        return ERROR;
    }

    res = coordinator_response.arg_1;

    if (res == ERROR){
        return ERROR;
    }

    return coordinator_response.arg_2;
}


/// MEJORAR ESTA IMPLEMENTACION
int32_t get_real_fd(int32_t operation, int32_t fd, int32_t server_pid){

    int is_fd_operation = (
        operation == OP_CLOSE || 
        operation == OP_READ  || 
        operation == OP_WRITE ||
        operation == OP_LSEEK || 
        operation == OP_DUP);

    if (!is_fd_operation){ 
        return ERROR;
    }

    int32_t coordinator_pid = get_coord_pid();
    int32_t res = send_msg(coordinator_pid, OP_GET_FD, server_pid, fd, 0, 0, 0);
    if (res == ERROR){
        return ERROR;
    }

    Msg coordinator_response;
    res = recv_msg(&coordinator_response);

    if (res == ERROR){
        return ERROR;
    }

    res = coordinator_response.arg_1;

    if (res == ERROR){
        return ERROR;
    }

    return coordinator_response.arg_2;
}


int32_t server_get_real_fd(int32_t fd, int32_t app_pid, int32_t type_server){

    int32_t coordinator_pid = get_coord_pid();
    int32_t res = send_msg(coordinator_pid, OP_GET_SERVER_FD, app_pid, fd, type_server, 0, 0);
    if (res == ERROR){
        return ERROR;
    }

    Msg coordinator_response;
    res = recv_msg(&coordinator_response);

    if (res == ERROR){
        return ERROR;
    }

    res = coordinator_response.arg_1;

    if (res == ERROR){
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
    //VER DE REFACTORIZAR LA SIGUIENTE IMPLEMENTACION

    int32_t real_fd = get_real_fd(arg_1, arg_2, server_pid);
    if (real_fd >= 0) {
        arg_2 = real_fd;
    }

    // ------------------------------------------------
    return send_msg(server_pid, arg_1, arg_2, arg_3, arg_4, arg_5, arg_6);
}


int32_t app_recv_fork_msg() {
    Msg msg;
    if (recv_msg(&msg) != SUCCESS) return ERROR;
    if (msg.arg_1 != APP_OP_FORK) return ERROR;

    int32_t father_pid = msg.arg_2;
    printf("[IPC-FORK] PID [%d] iniciando sincronizacion de herencia desde Padre [%d]\n", getpid(), father_pid);

    printf("[IPC-FORK] Notificando al Coordinador...\n");
    send_msg(get_coord_pid(), OP_FORK, father_pid, 0, 0, 0, 0);
    app_receive_parameter(OP_FORK);

    int32_t fs_pid = get_server_pid(OP_OPEN, 0, 0, 0, 0, 0);
    if (fs_pid >= 0) {
        printf("[IPC-FORK] Notificando al Filesystem (PID %d)...\n", fs_pid);
        send_msg(fs_pid, OP_FORK, 0, father_pid, 0, 0, 0);
        app_receive_parameter(OP_FORK);
    }

    int32_t pipe_pid = get_server_pid(OP_PIPE, 0, 0, 0, 0, 0);
    if (pipe_pid >= 0) {
        printf("[IPC-FORK] Notificando al Pipe (PID %d)...\n", pipe_pid);
        send_msg(pipe_pid, OP_FORK, 0, father_pid, 0, 0, 0);
        app_receive_parameter(OP_FORK);
    }
    
    printf("[IPC-FORK] Sincronizacion completa. Desbloqueando Padre [%d]\n", father_pid);
    send_msg(father_pid, APP_OP_FORK, SUCCESS, 0, 0, 0, 0);
    return SUCCESS;
}
int32_t app_send_fork_msg(int32_t child_pid) {
    int32_t father_pid = getpid();
    printf("father pid [%d] child pid [%d]\n", father_pid, child_pid);
    int32_t res = send_msg(child_pid, APP_OP_FORK, father_pid, 0, 0, 0, 0);
    
    if (res < 0) {
        return ERROR;
    }

    Msg ack_msg;
    if (recv_msg(&ack_msg) != SUCCESS) {
        return ERROR;
    }

    return SUCCESS;
}


int32_t app_send_close_msg() {
    int32_t fs_pid = get_server_pid(OP_CLOSE_ALL, 0, FILESYSTEM, 0, 0, 0);
    if (fs_pid >= 0) {
        send_msg(fs_pid, OP_CLOSE_ALL, 0, 0, 0, 0, 0);
        app_receive_parameter(OP_CLOSE_ALL);
    }

    int32_t pipe_pid = get_server_pid(OP_CLOSE_ALL, 0, PIPE, 0, 0, 0);
    if (pipe_pid >= 0) {
        send_msg(pipe_pid, OP_CLOSE_ALL, 0, 0, 0, 0, 0);
        app_receive_parameter(OP_CLOSE_ALL);
    }

    return SUCCESS;
}
