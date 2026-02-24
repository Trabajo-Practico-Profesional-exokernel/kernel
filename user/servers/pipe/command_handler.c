#include "command_handler.h"
#include "ipc.h"
#include "inc/operations.h"
#include "types.h"
#include "console/debug.h"
#include "constants.h"
#include "syscalls.h"
#include "files.h"
#include "arch/proc.h"

extern struct File files[PROCS_MAX][MAX_FILES];

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
        case OP_FORK:   return PIPE_OP_FORK;
        case OP_CLOSE_ALL: return PIPE_OP_CLOSE_ALL;
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
        case PIPE_OP_FORK:   return OP_FORK;
        case PIPE_OP_CLOSE_ALL: return OP_CLOSE_ALL;
        default:             return -1;
    }
}

static int32_t update_coord_state(int32_t type_server, int32_t type_command, int32_t client_pid, int32_t fd, int fd_2){
    if (type_command != OP_OPEN && type_command != OP_CLOSE && type_command != OP_PIPE){
        return -1;
    }
    
    int32_t coord_pid = get_coord_pid();
    int32_t state = 0;

    if (type_command == OP_OPEN || type_command == OP_PIPE || type_command == OP_DUP){
        state = 1;
    } else if (type_command == OP_CLOSE){
        state = -1;
    } else {
        return SUCCESS;
    }

    printf("[PIPE] update_coord_state: Notificando Coordinador PID[%d] -> FD Real[%d], State[%d], extra: %d\n", client_pid, fd, state, fd_2);
    return send_msg(coord_pid, OP_UPDATE, getpid(), client_pid, fd, state, fd_2);
}

int32_t get_command(PipeOperation *command) {
    Msg msg;

    if (recv_msg(&msg) != SUCCESS) {
        return ERROR;
    }

    current_client_pid = msg.sender_pid;

    command->app_id = msg.sender_pid;
    command->type_op = map_op_code(msg.arg_1);

    command->fd            = msg.arg_2;
    command->arg_1         = msg.arg_3; 
    command->content_vaddr = msg.arg_5;
    command->len_content   = msg.arg_6;

    return SUCCESS;
}



int32_t give_response(int32_t type_command, int32_t arg_1, int32_t arg_2, int32_t arg_3, int32_t fd_1, int32_t fd_2){
    int32_t protocol_op = unmap_op_code(type_command);
    
    if (current_client_pid == -1 || protocol_op == -1) {
        return ERROR;
    }
    
    if ((type_command == PIPE_OP_OPEN || type_command == PIPE_OP_READ) && arg_2 != 0) {
        if (arg_2 != 0 && arg_3 > 0 && arg_1 >= 0) {
             if (protocol_op == OP_PIPE) {
                update_coord_state(PIPE, OP_OPEN, current_client_pid, fd_1, 0);
                update_coord_state(PIPE, OP_OPEN, current_client_pid, fd_2, 0);

                int32_t real_fd_1 = server_get_real_fd(fd_1, current_client_pid, PIPE);
                int32_t real_fd_2 = server_get_real_fd(fd_2, current_client_pid, PIPE);
                int32_t real_fds[] = {real_fd_1, real_fd_2};
                return server_send_content_to_app(current_client_pid, protocol_op, (char*)real_fds, arg_3, arg_1);
             } else {
                return server_send_content_to_app(current_client_pid, protocol_op, (char*)arg_2, arg_3, arg_1);
             }
        }
        return server_send_parameter_to_app(current_client_pid, protocol_op, arg_1);
    } 
    else {
        if (arg_1 >= 0) {
            if (protocol_op == OP_CLOSE) {
                update_coord_state(PIPE, protocol_op, current_client_pid, fd_1, 0);
            }
            if (protocol_op == OP_DUP) {
                update_coord_state(PIPE, OP_OPEN, current_client_pid, arg_1, fd_2);
                int32_t real_fd_1 = server_get_real_fd(arg_1, current_client_pid, PIPE);
                return server_send_parameter_to_app(current_client_pid, protocol_op, real_fd_1);
            }
            if (protocol_op == OP_CLOSE_ALL) {
                for (int i = 0; i < MAX_FILES; i++) {
                    if (files[current_client_pid][i].state == ON) {
                        update_coord_state(PIPE, OP_CLOSE, current_client_pid, i, 0);
                    }
                }
            }
        }
        return server_send_parameter_to_app(current_client_pid, protocol_op, arg_1);
    }
}
