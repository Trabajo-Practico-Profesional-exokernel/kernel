#include "command_handler.h"
#include "ipc.h"
#include "inc/operations.h"
#include "types.h"
#include "console/debug.h"
#include "constants.h"
#include "syscalls.h"

static int32_t current_client_pid = -1;

void reset_current_client_pid() {
    current_client_pid = -1;
}

static int32_t map_op_code(int32_t protocol_op) {
    switch(protocol_op) {
        case OP_PIPE:   return PIPE_OP_OPEN;
        case OP_READ:   return PIPE_OP_READ;
        case OP_WRITE:  return PIPE_OP_WRITE;
        case OP_CLOSE:  return PIPE_OP_CLOSE;
        case OP_STAT:   return PIPE_OP_FSTAT;
        case OP_DUP:    return PIPE_OP_DUP;
        default:        return -1;
    }
}

static int32_t unmap_op_code(int32_t pipe_op) {
    switch(pipe_op) {
        case PIPE_OP_OPEN:   return OP_PIPE;
        case PIPE_OP_READ:   return OP_READ;
        case PIPE_OP_WRITE:  return OP_WRITE;
        case PIPE_OP_CLOSE:  return OP_CLOSE;
        case PIPE_OP_FSTAT:  return OP_STAT;
        case PIPE_OP_DUP:    return OP_DUP;
        default:             return -1;
    }
}

static int32_t update_coord_state(int32_t type_server, int32_t type_command, int32_t client_pid, int32_t fd){
    
    if (type_command != OP_OPEN && type_command != OP_CLOSE && type_command != OP_PIPE){
        return -1;
    }
    
    int32_t coord_pid = get_coord_pid();
    int32_t state = 0;

    if (type_command == OP_OPEN || type_command == OP_PIPE){
        state = 1; 
    } 
    else if (type_command == OP_CLOSE){
        state = -1;
    } 
    else {
        return SUCCESS;
    }

    return send_msg(coord_pid, OP_UPDATE, getpid(), client_pid, fd, state, 0);
}

int32_t get_command(PipeOperation *command){
    Msg msg;

    if (recv_msg(&msg) != SUCCESS) {
        return ERROR;
    }

    current_client_pid = msg.sender_pid;

    command->app_id = msg.sender_pid;
    command->type_op = map_op_code(msg.arg_1);

    command->fd            = msg.arg_2;
    command->content_vaddr = msg.arg_5;
    command->len_content   = msg.arg_6;

    return SUCCESS;
}

int32_t give_response(int32_t type_command, int32_t arg_1, int32_t arg_2, int32_t arg_3, int32_t fd_1, int32_t fd_2){

    int32_t protocol_op = unmap_op_code(type_command);

    if (current_client_pid == -1 || protocol_op == -1) {
        return ERROR;
    }
    
    if ((type_command == PIPE_OP_OPEN || 
        type_command == PIPE_OP_READ) && arg_2 != 0)
    {
        // Si se espera contenido
        if (arg_2 != 0 && arg_3 > 0 && arg_1 > 0) {
            return server_send_content_to_app(current_client_pid, protocol_op, (char*)arg_2, arg_3);
        }
        // Si fallo (ej. bytes_read < 0), enviamos solo el codigo de error
        return server_send_parameter_to_app(current_client_pid, protocol_op, arg_1);
    } 
    else {
        // Respuesta estandar (exito/error o valor entero como FD)
        if (arg_1 >= 0) {
            if (protocol_op == OP_CLOSE || protocol_op == OP_DUP) {
                update_coord_state(PIPE, protocol_op, current_client_pid, fd_1);
            }
            if (protocol_op == OP_OPEN) {
                update_coord_state(PIPE, protocol_op, current_client_pid, fd_2);
            }
        }
        return server_send_parameter_to_app(current_client_pid, protocol_op, arg_1);
    }
}
