#include "coordinator.h"
#include "inc/operations.h"
#include "types.h"
#include "ipc_msgs.h"
#include "constants.h"
#include "syscalls.h"
#include "../../meta/app_names.h"
#include "ipc.h"
#include "server_map.h"
#include "string.h"
#include "stdio.h"

Coordinator coordinator;

static const char * const server_names[] = {
    [COORD] = "coordinator",
    [FILESYSTEM]  = "filesystem",
    [PIPE]        = "pipe",
    [CONSOLE]     = "console",
    [SHELL] = "shell"
};

#define GET_SERVER_NAME(type) \
    ((type >= 0 && type < SERVER_COUNT) ? server_names[type] : "unknown")

extern char* _app_names[];

int32_t start_server(Server type) {

    char* program_name = (char*)GET_SERVER_NAME(type);

    int len_name = strlen((const uint8_t*)program_name) + 1;
    for (int ind_program = 0; ind_program < APP_COUNT; ind_program++) {
        
        if (strncmp((const uint8_t*)program_name, (const uint8_t*)_app_names[ind_program], len_name) == 0) {
            char* argv[3];
            argv[0] = program_name;
            argv[1] = 0;
            argv[2] = 0;

            int proc_pid = exec(ind_program, argv);
            if (proc_pid < 0) {
                printf((const uint8_t*)"[Coordinator] Failed to start %s. Error: %d\n", program_name, proc_pid);
                return ERROR;
            }

            printf((const uint8_t*)"[Coordinator] %s started with PID: %d.\n", program_name, proc_pid);
            server_map_set(&coordinator.server_map, type, proc_pid);

            return SUCCESS;
        }
    }
    return ERROR;
}


#define UNUSED(x) (void)(x)

// NOTA: 'CoordinatorMsg' es un typedef, no usar 'struct CoordinatorMsg'

void handler_noop(CoordinatorMsg *op){
    UNUSED(op);
}


void handler_op(CoordinatorMsg *op){
    UNUSED(op);
}

void handler_kernel_op(CoordinatorMsg *op){
    UNUSED(op);
}

void handler_fs_op(CoordinatorMsg *op){
    uint32_t fs_pid;
    fs_pid = server_map_get(&coordinator.server_map, FILESYSTEM);
    int32_t result;
    
    if (alive(fs_pid)){
        result = SUCCESS;
    } else {
        result = start_server(FILESYSTEM);
        fs_pid = server_map_get(&coordinator.server_map, FILESYSTEM);
    }

    if (result == SUCCESS) {
        send_msg_to_app(op->app_id, SUCCESS, fs_pid, FILESYSTEM);
    } else {
        send_msg_to_app(op->app_id, ERROR, 0, 0);
    }
}

void handler_pipe_op(CoordinatorMsg *op){
    UNUSED(op);
}

void handler_generic_op(CoordinatorMsg *op){
    UNUSED(op);
}

#define MAX_HANDLERS (sizeof(dispatch_table) / sizeof(dispatch_table[0]))

typedef void (*op_handler_t)(CoordinatorMsg *);

static const op_handler_t dispatch_table[] = {
    [OP_NOOP]           = handler_noop,
    [OP_EXIT]           = handler_noop,
    [OP_EXEC]           = handler_noop,
    [OP_WAIT]           = handler_noop,
    [OP_YIELD]          = handler_noop,
    [OP_GETPID]         = handler_noop,
    [OP_UPTIME]         = handler_noop,
    [OP_SBRK]           = handler_noop,
    [OP_PUTCHAR]        = handler_noop,
    [OP_GETCHAR]        = handler_noop,
    [OP_OPEN]           = handler_fs_op,
    [OP_CLOSE]          = handler_fs_op,
    [OP_READ]           = handler_fs_op,
    [OP_WRITE]          = handler_fs_op,
    [OP_LSEEK]          = handler_fs_op,
    [OP_FSTAT]          = handler_fs_op,
    [OP_DUP]            = handler_noop,
    [OP_PIPE]           = handler_noop,
    [OP_MKDIR]          = handler_fs_op,
    [OP_RMDIR]          = handler_fs_op,
    [OP_CHDIR]          = handler_fs_op,
    [OP_PWD]            = handler_fs_op,
    [OP_LS]             = handler_fs_op,
    [OP_MKNOD]          = handler_fs_op,
    [OP_LINK]           = handler_fs_op,
    [OP_UNLINK]         = handler_fs_op,
    [OP_CHOWN]          = handler_fs_op,
    [OP_CHMOD]          = handler_fs_op,
    [OP_DISK_READ]      = handler_noop,
    [OP_DISK_WRITE]     = handler_noop,
    [OP_REG_HANDLER]    = handler_noop,
    [OP_HANDLER_RET]    = handler_noop,
};

void init_coordinator(){
    server_map_init(&coordinator.server_map);
    server_map_set(&coordinator.server_map, COORD, getpid());
}

void init_servers(){
    start_server(SHELL);
}

void send_int_response(int pid_target, int value) {
    sys_try_send_msg(pid_target, (char *)&value, 1);
}

void send_response_to_app(int32_t app_id, int32_t arg_1){
    AppMsg msg;
    // Mapeo: arg_1 (resultado) -> arg_1, server_type (COORD) -> type_msg
    msg.arg_1 = arg_1;
    msg.type_msg = COORD; 
    msg.arg_2 = 0;

    sys_try_send_content(app_id, (char*)&msg, sizeof(AppMsg));
}

void dispatch_request(CoordinatorMsg *msg) {
    if (msg->type_msg >= 0 && msg->type_msg < MAX_HANDLERS && dispatch_table[msg->type_msg]) {
        dispatch_table[msg->type_msg](msg);
    } else {
        send_msg_to_app(msg->app_id, ERROR, 0, 0);
    }
}


void server_listen() {
    CoordinatorMsg msg;
    while (1) {
        int res = sys_recv_content((char*)&msg, sizeof(CoordinatorMsg));
        if (res == SUCCESS) {
            dispatch_request(&msg);
        }
    }
}
