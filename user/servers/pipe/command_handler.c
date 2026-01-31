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
        case PIPE_OP_OPEN:   return OP_OPEN;
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

int32_t give_response(int32_t type_command, int32_t arg_1, int32_t arg_2, int32_t arg_3){
    return 0;
}
/*
int32_t give_response(int32_t type_command, int32_t arg_1, int32_t arg_2, int32_t arg_3){
    // Nota: arg_3 en FS era el 'size' solicitado original, arg_2 el buffer.
    // Aquí asumimos una firma similar: arg_1=resultado/bytes, arg_2=buffer/data, arg_3=len_solicitada

    int32_t protocol_op = unmap_op_code(type_command);

    if (current_client_pid == -1 || protocol_op == -1) {
        return ERROR;
    }

    if (type_command == PIPE_OP_READ) {
        // arg_1: Bytes leídos efectivamente
        // arg_2: Puntero al buffer con datos
        // arg_3: Bytes solicitados por el usuario
        
        if (arg_2 != 0 && arg_3 > 0 && arg_1 > 0) {
            return server_send_content_to_app(current_client_pid, protocol_op, (char*)arg_2, arg_1);
        }
        // Si no se leyó nada o hubo error, enviamos el código/cantidad (ej: 0 para EOF o negativo error)
        return server_send_parameter_to_app(current_client_pid, protocol_op, arg_1);
    } 
    
    // --- CASO 2: OTRAS OPERACIONES (Write, Close, Open, Dup) ---
    else {
        // Si la operación fue exitosa (arg_1 >= 0), verificamos si hay que actualizar al coordinador
        if (arg_1 >= 0) {
            
            // Si cerramos un pipe, hay que avisar al coordinador para liberar el FD
            if (protocol_op == OP_CLOSE) {
                // arg_3 suele traer el FD en give_response del FS si se pasa explícitamente, 
                // o asumimos que quien llama a give_response tiene el FD a mano.
                // Como give_response no recibe el FD explícito en tu firma (fd estaba en get_command),
                // asumiremos que arg_2 o arg_1 contiene el dato relevante o modificamos la firma.
                
                // NOTA: En tu FS `give_response` recibe `int32_t fd` como último parámetro.
                // En tu definición solicitada para Pipe es `int32_t arg_3`.
                // Asumiré que para OP_CLOSE, el `fd` cerrado debe venir en alguno de los args 
                // para poder pasarlo a update_coord_state. 
                // Por convención del FS, pasaremos el FD en la llamada a give_response.
                
                // *IMPORTANTE*: Si la firma es estricta (arg_1, arg_2, arg_3), 
                // asegúrate de pasar el FD en uno de ellos al llamar a esta función desde main.c.
                // Asumiremos arg_2 = fd para CLOSE en este ejemplo.
                
                update_coord_state(PIPE, protocol_op, current_client_pid, arg_2);
            }
            
            else if (protocol_op == OP_OPEN || protocol_op == OP_PIPE) {
                 update_coord_state(PIPE, protocol_op, current_client_pid, arg_1);
            }
        }

        return server_send_parameter_to_app(current_client_pid, protocol_op, arg_1);
    }
}*/
