#include "command_handler.h"
#include "ipc.h"
#include "inc/operations.h"
#include "types.h"
#include "console/debug.h"
#include "constants.h"
#include "command_handler.h"
#include "ipc.h"
#include "inc/operations.h"
#include "types.h"
#include "console/debug.h"
#include "syscalls.h"
#include "fs.h"
#include "direct_printf.h"

extern FileDescriptor table[PROCS_MAX][MAX_OPEN_FILES];

static int32_t current_client_pid = -1;

static int32_t map_op_code(int32_t protocol_op) {
    switch(protocol_op) {
        case OP_OPEN:   return FS_OP_OPEN;
        case OP_CLOSE:  return FS_OP_CLOSE;
        case OP_READ:   return FS_OP_READ;
        case OP_WRITE:  return FS_OP_WRITE;
        case OP_LSEEK:  return FS_OP_LSEEK;
        case OP_STAT:   return FS_OP_FSTAT;
        case OP_DUP:    return FS_OP_DUP;
        case OP_PIPE:   return FS_OP_PIPE;
        case OP_MKDIR:  return FS_OP_MKDIR;
        case OP_RMDIR:  return FS_OP_RMDIR;
        case OP_CHDIR:  return FS_OP_CHDIR;
        case OP_CWD:    return FS_OP_PWD;
        case OP_LS:     return FS_OP_LS;
        case OP_MKNOD:  return FS_OP_MKNOD;
        case OP_LINK:   return FS_OP_LINK;
        case OP_UNLINK: return FS_OP_UNLINK;
        case OP_CHOWN:  return FS_OP_CHOWN;
        case OP_CHMOD:  return FS_OP_CHMOD;
        case OP_FORK:   return FS_OP_FORK;
        case OP_CLOSE_ALL: return FS_OP_CLOSE_ALL;
        default:        return -1;
    }
}

static int32_t unmap_op_code(int32_t fs_op) {
    switch(fs_op) {
        case FS_OP_OPEN:   return OP_OPEN;
        case FS_OP_CLOSE:  return OP_CLOSE;
        case FS_OP_READ:   return OP_READ;
        case FS_OP_WRITE:  return OP_WRITE;
        case FS_OP_LSEEK:  return OP_LSEEK;
        case FS_OP_FSTAT:  return OP_STAT;
        case FS_OP_DUP:    return OP_DUP;
        case FS_OP_PIPE:   return OP_PIPE;
        case FS_OP_MKDIR:  return OP_MKDIR;
        case FS_OP_RMDIR:  return OP_RMDIR;
        case FS_OP_CHDIR:  return OP_CHDIR;
        case FS_OP_PWD:    return OP_CWD; 
        case FS_OP_LS:     return OP_LS;
        case FS_OP_MKNOD:  return OP_MKNOD;
        case FS_OP_LINK:   return OP_LINK;
        case FS_OP_UNLINK: return OP_UNLINK;
        case FS_OP_CHOWN:  return OP_CHOWN;
        case FS_OP_CHMOD:  return OP_CHMOD;
        case FS_OP_FORK:   return OP_FORK;
        case FS_OP_CLOSE_ALL: return OP_CLOSE_ALL;
        default:           return -1;
    }
}

int32_t get_command(FilesystemOperation *command){
    Msg msg;

    if (recv_msg(&msg) != SUCCESS) {
        return ERROR;
    }
    // se guarda el pid actual del emisor para la respuesta de vuelta
    current_client_pid = msg.sender_pid;

    command->app_id = msg.sender_pid;
    command->type_op = map_op_code(msg.arg_1);

    // direct_printf("FS GOT ARGS arg1: %d, arg2: %d, arg3: %d, arg4: %d, arg5: %d, arg6: %d\n",
    //      msg.arg_1,msg.arg_2,msg.arg_3,msg.arg_4,msg.arg_5,msg.arg_6);
    //caso especial para link
    if (command->type_op == FS_OP_LINK) {
        command->fd = 0;
        command->arg_1 = 0;
        command->arg_2 = 0;
        
        command->content_1_vaddr = msg.arg_2;
        command->len_content_1   = msg.arg_3;
        
        command->content_2_vaddr = msg.arg_4;
        command->len_content_2   = msg.arg_5;
    } 
    else {
        // caso estandar para las demas operaciones

        command->fd    = msg.arg_2;
        command->arg_1 = msg.arg_3;
        command->arg_2 = msg.arg_4;
        
        command->content_1_vaddr = msg.arg_5;
        command->len_content_1   = msg.arg_6;
        
        command->content_2_vaddr = 0;
        command->len_content_2   = 0;
    }

    return SUCCESS;
}


int32_t update_coord_state(int32_t type_server, int32_t type_command, int32_t current_client_pid, int32_t fd,int32_t fd_2 ){
    
    if (type_command != OP_OPEN && type_command != OP_CLOSE && type_command != OP_DUP){
        return -1;
    }
    
    int32_t coord_pid = get_coord_pid();
    int32_t state = 0;

    if (type_command == OP_OPEN || type_command == OP_DUP){
        state = 1; 
    } else if (type_command == OP_CLOSE){
        state = -1;
    } else {
        return SUCCESS;
    }

    //direct_printf("[FS] SEND UPD FD TO COORD %d to %d, state %d\n", fd, fd_2, state);
    return send_msg(coord_pid, OP_UPDATE, getpid(), current_client_pid, fd, state, fd_2);
}

int32_t give_response(int32_t type_command, int32_t arg_1, int32_t arg_2, int32_t arg_3, int32_t fd, int32_t fd_2){

    int32_t protocol_op = unmap_op_code(type_command);

    if (current_client_pid == -1 || protocol_op == -1) {
        return ERROR;
    }
    
    if (type_command == FS_OP_READ || 
        /*type_command == FS_OP_FSTAT || */
        type_command == FS_OP_PWD || 
        (type_command == FS_OP_LS && arg_2 != 0))
    {
        // Si se espera contenido
        if (arg_2 != 0 && arg_3 > 0 && arg_1 > 0 || type_command == FS_OP_PWD) {
            return server_send_content_to_app(current_client_pid, protocol_op, (char*)arg_2, arg_3, arg_1);
        }
        // Si fallo (ej. bytes_read < 0), enviamos solo el codigo de error
        return server_send_parameter_to_app(current_client_pid, protocol_op, arg_1);
    } 
    else {
        // Respuesta estandar (exito/error o valor entero como FD)
        if (arg_1 >= 0) {
            if (protocol_op == OP_OPEN || protocol_op == OP_CLOSE) {
                update_coord_state(FILESYSTEM, protocol_op, current_client_pid, fd, -1);
            }
            if (protocol_op == OP_OPEN ){
                int32_t real_fd = server_get_real_fd(arg_1, current_client_pid, FILESYSTEM);
                //direct_printf("[GOT FD] %d for fs fd %d\n", real_fd, fd);
                return server_send_parameter_to_app(current_client_pid, protocol_op, real_fd);
            }
            if (protocol_op == OP_DUP) {
                update_coord_state(FILESYSTEM, OP_OPEN, current_client_pid, arg_1, fd_2);
                int32_t real_fd = server_get_real_fd(arg_1, current_client_pid, FILESYSTEM);
                return server_send_parameter_to_app(current_client_pid, protocol_op, real_fd);
            }
            /*if (protocol_op == OP_FORK) {
                for (int i = 0; i < MAX_OPEN_FILES; i++) {
                    if (table[current_client_pid][i].fd != -1) {
                        update_coord_state(FILESYSTEM, OP_OPEN, current_client_pid, i, -1);
                    }
                }
            }*/
            if (protocol_op == OP_CLOSE_ALL) {
                for (int i = 0; i < MAX_OPEN_FILES; i++) {
                    if (table[current_client_pid][i].fd != -1) {
                        update_coord_state(FILESYSTEM, OP_CLOSE, current_client_pid, i, -1);
                    }
                }
            }
        }
        return server_send_parameter_to_app(current_client_pid, protocol_op, arg_1);
    }
}
