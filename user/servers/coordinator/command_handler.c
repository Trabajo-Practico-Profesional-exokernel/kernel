
#include "command_handler.h"
#include "ipc.h"
#include "inc/operations.h"
#include "types.h"
#include "console/debug.h"
#include "constants.h"


static int32_t current_client_pid = -1;

int32_t get_command(CoordinatorOperation *command) {
    Msg msg;

    if (recv_msg(&msg) != SUCCESS) {
        return ERROR;
    }

    command->type_op = msg.arg_1;

    if (command->type_op == OP_UPDATE) {
        command->server_type = msg.arg_2; 
        command->app_id      = msg.arg_3;
        command->fd          = msg.arg_4;
        command->state       = msg.arg_5;
        return SUCCESS;
    } 
    
    if (command->type_op == OP_GET_FD) {
        command->app_id      = msg.sender_pid;
        command->server_type = msg.arg_2;
        command->fd          = msg.arg_3;
        return SUCCESS;
    } 
    
    
    current_client_pid = msg.sender_pid;
    command->app_id      = msg.sender_pid;
    command->fd          = msg.arg_2;          
    command->server_type = msg.arg_3; 
    

    return SUCCESS;
}

int32_t give_response(int32_t type_command, int32_t arg_1, int32_t arg_2, int32_t arg_3) {
    if (current_client_pid == -1) {
        return ERROR;
    }

    return server_send_parameter_to_app(current_client_pid, type_command, arg_1);
}

void reset_current_client_pid(){
    current_client_pid = -1;
}
