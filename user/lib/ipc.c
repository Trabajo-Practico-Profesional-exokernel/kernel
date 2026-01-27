#include "ipc.h"
#include "syscalls.h" 
#include "string.h"
#include "lib.h" 
#include "constants.h"
#include "inc/operations.h"
#include "ipc_msgs.h"


int32_t get_service_data(int32_t *service, int32_t sender_pid, int32_t type_msg, 
            int32_t arg_1, int32_t arg_2, int32_t arg_3, 
            void* content, int32_t len_content){
    int32_t result;
    int32_t coord_pid = get_coord_pid();
    
    CoordinatorMsg msg;
    msg.app_id = sender_pid;
    msg.type_msg = type_msg;
    msg.arg_1 = arg_1;

    result = sys_try_send_content(coord_pid, (char*)&msg, sizeof(CoordinatorMsg));
    if (result == ERROR){
        return ERROR;
    }

    AppMsg msg_received;
    result = sys_try_recv_content((char*)&msg_received, sizeof(AppMsg));

    if (result == ERROR){
        return ERROR;
    }

    service[0] = msg_received.arg_1;
    service[1] = msg_received.arg_2;
    return SUCCESS;
}

int send_msg_to_service(int32_t type_msg, 
                 int32_t arg_1, int32_t arg_2, int32_t arg_3, 
                 void* content, int32_t len_content) {

    int32_t service[2];
    int32_t sender_pid = getpid();
    int32_t result = get_service_data(service, sender_pid, 
                    type_msg, arg_1, arg_2, arg_3,
                    content, len_content);
    if (result == ERROR) {
        return ERROR;
    }
    int32_t dest_server = service[1];
    int32_t dest_pid = service[0]; // Usar el PID resuelto

    switch (dest_server) {
        case COORD: {
            CoordinatorMsg msg;
            msg.app_id = sender_pid;
            msg.type_msg = type_msg;
            msg.arg_1 = arg_1;
            
            result = sys_try_send_content(get_coord_pid(), (char*)&msg, sizeof(CoordinatorMsg));
            break;
        }

        case FILESYSTEM: {
            FilesystemMsg msg;
            memset(&msg, 0, sizeof(FilesystemMsg));
            
            msg.app_id = sender_pid;
            msg.type_msg = type_msg;
            msg.arg_1 = arg_1;
            msg.arg_2 = arg_2;
            msg.arg_3 = arg_3;
            if (content && len_content > 0) {
                int32_t copy_len = (len_content > MAX_CONTENT_SIZE) ? MAX_CONTENT_SIZE : len_content;
                memcpy(msg.content, content, copy_len);
            }

            result = sys_try_send_content(dest_pid, (char*)&msg, sizeof(FilesystemMsg));
            break;
        }

        default:
            return ERROR;
    }

    return result;
}


int32_t send_msg_to_app(int32_t app_id, int32_t type_msg, int32_t arg_1, int32_t arg_2){
    AppMsg msg;
    msg.type_msg = type_msg;
    msg.arg_1 = arg_1;
    msg.arg_2 = arg_2;
    int32_t result = sys_try_send_content(app_id, (char*)&msg, sizeof(AppMsg));
    return result;
}

int32_t receive_msg(ServiceMsgUnion *buffer) {
    if (!buffer) {
        return ERROR;
    }

    return sys_recv_content((char*)buffer, sizeof(ServiceMsgUnion));
}


int ping(int32_t server_pid, int32_t server_type){
    int32_t sender_pid = getpid();
    int32_t result;
    switch (server_type) {
        case FILESYSTEM: {
            FilesystemMsg msg;
            memset(&msg, 0, sizeof(FilesystemMsg));
            
            msg.app_id = sender_pid;
            msg.type_msg = FS_OP_PING;

            result = sys_try_send_content(server_pid, (char*)&msg, sizeof(FilesystemMsg));
            break;
        }

        default:
            return ERROR;
    }


    return result;
}