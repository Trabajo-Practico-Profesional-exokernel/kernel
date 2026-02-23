#include "command_handler.h"
#include "ipc.h"
#include "inc/operations.h"
#include "types.h"
#include "console/debug.h"
#include "constants.h"
#include "coordinator.h"

void handle_putchar(CoordinatorOperation *op) {
    int32_t res = coordinator_putchar();
    give_response(op->type_op, res, 0, 0);
}

void handle_getchar(CoordinatorOperation *op) {
    int32_t res = coordinator_getchar();
    give_response(op->type_op, res, 0, 0);
}

void handle_open(CoordinatorOperation *op) {
    int32_t res = coordinator_open();
    give_response(op->type_op, res, 0, 0);
}

void handle_close(CoordinatorOperation *op) {
    int32_t res = coordinator_close(op->fd, op->app_id);
    give_response(op->type_op, res, 0, 0);
}

void handle_read(CoordinatorOperation *op) {
    int32_t res = coordinator_read(op->fd, op->app_id);
    give_response(op->type_op, res, 0, 0);
}

void handle_write(CoordinatorOperation *op) {
    int32_t res = coordinator_write(op->fd, op->app_id);
    give_response(op->type_op, res, 0, 0);
}

void handle_lseek(CoordinatorOperation *op) {
    int32_t res = coordinator_lseek(op->fd, op->app_id);
    give_response(op->type_op, res, 0, 0);
}

void handle_stat(CoordinatorOperation *op) {
    int32_t res = coordinator_stat();
    give_response(op->type_op, res, 0, 0);
}

void handle_dup(CoordinatorOperation *op) {
    int32_t res = coordinator_dup(op->fd, op->app_id);
    give_response(op->type_op, res, 0, 0);
}

void handle_pipe(CoordinatorOperation *op) {
    int32_t res = coordinator_pipe();
    give_response(op->type_op, res, 0, 0);
}

void handle_mkdir(CoordinatorOperation *op) {
    int32_t res = coordinator_mkdir();
    give_response(op->type_op, res, 0, 0);
}

void handle_rmdir(CoordinatorOperation *op) {
    int32_t res = coordinator_rmdir();
    give_response(op->type_op, res, 0, 0);
}

void handle_chdir(CoordinatorOperation *op) {
    int32_t res = coordinator_chdir();
    give_response(op->type_op, res, 0, 0);
}

void handle_cwd(CoordinatorOperation *op) {
    int32_t res = coordinator_cwd();
    give_response(op->type_op, res, 0, 0);
}

void handle_ls(CoordinatorOperation *op) {
    int32_t res = coordinator_ls();
    give_response(op->type_op, res, 0, 0);
}

void handle_mknod(CoordinatorOperation *op) {
    int32_t res = coordinator_mknod();
    give_response(op->type_op, res, 0, 0);
}

void handle_link(CoordinatorOperation *op) {
    int32_t res = coordinator_link();
    give_response(op->type_op, res, 0, 0);
}

void handle_unlink(CoordinatorOperation *op) {
    int32_t res = coordinator_unlink();
    give_response(op->type_op, res, 0, 0);
}

void handle_chown(CoordinatorOperation *op) {
    int32_t res = coordinator_chown();
    give_response(op->type_op, res, 0, 0);
}

void handle_chmod(CoordinatorOperation *op) {
    int32_t res = coordinator_chmod();
    give_response(op->type_op, res, 0, 0);
}


void handle_update(CoordinatorOperation *op) {
    coordinator_update(op->fd, op->app_id, op->server_type, op->state);
}

void send_error_msg(int32_t operation) {
    give_response(operation, ERROR, 0, 0);
}

void handle_get_fd(CoordinatorOperation *op) {
    int32_t res = get_server_fd(op->app_id, op->fd, op->server_type);
    give_response(op->type_op, res, 0, 0);
}

void handle_get_server_fd(CoordinatorOperation *op) {
    int32_t res = get_server_real_fd(op->app_id, op->fd, op->server_type);
    give_response(op->type_op, res, 0, 0);
}

void handle_fork(CoordinatorOperation *op) {
    int32_t child_pid = op->app_id;
    int32_t father_pid = op->fd; 
    
    int32_t res = coordinator_fork(child_pid, father_pid);
    give_response(op->type_op, res, 0, 0);
}

void handle_close_all(CoordinatorOperation *op) {
    int32_t res = coordinator_close_all(op->server_type);
    give_response(op->type_op, res, 0, 0);
}

void handle_noop() {
    reset_current_client_pid();
}

typedef void (*coord_op_handler_t)(CoordinatorOperation *op);

static const coord_op_handler_t op_dispatch_table[] = {
    [OP_NOOP]           = handle_noop,
    [OP_OPEN]           = handle_open,
    [OP_CLOSE]          = handle_close,
    [OP_READ]           = handle_read,
    [OP_WRITE]          = handle_write,
    [OP_LSEEK]          = handle_lseek,
    [OP_STAT]           = handle_stat,
    [OP_DUP]            = handle_dup,
    [OP_PIPE]           = handle_pipe,
    [OP_MKDIR]          = handle_mkdir,
    [OP_RMDIR]          = handle_rmdir,
    [OP_CHDIR]          = handle_chdir,
    [OP_CWD]            = handle_cwd,
    [OP_LS]             = handle_ls,
    [OP_MKNOD]          = handle_mknod,
    [OP_LINK]           = handle_link,
    [OP_UNLINK]         = handle_unlink,
    [OP_CHOWN]          = handle_chown,
    [OP_CHMOD]          = handle_chmod,
    [OP_UPDATE]         = handle_update,
    [OP_GET_FD]         = handle_get_fd,
    [OP_GET_SERVER_FD]  = handle_get_server_fd,
    [OP_FORK]           = handle_fork,
    [OP_CLOSE_ALL]      = handle_close_all,
};

#define MAX_OP_HANDLERS (sizeof(op_dispatch_table) / sizeof(op_dispatch_table[0]))

void dispatch_command(CoordinatorOperation *op) {
    if (op->type_op >= 0 && op->type_op < MAX_OP_HANDLERS && op_dispatch_table[op->type_op]) {
        op_dispatch_table[op->type_op](op);
    } else {
        // Operación no reconocida o fuera de rango
        send_error_msg(op->type_op);
    }
}

void exec_command(CoordinatorOperation command) {
    dispatch_command(&command);
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    disable_debug_print();
    init_coordinator();
    init_servers();

    CoordinatorOperation command;

    while(1){
        if (get_command(&command) == SUCCESS) {
            exec_command(command);
        }
    }
    
    return 0;
}
