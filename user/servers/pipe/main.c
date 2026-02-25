#include "command_handler.h"
#include "ipc.h"
#include "inc/operations.h"
#include "types.h"
#include "console/debug.h"
#include "constants.h"
#include "pipe.h"
#include "files.h"
#include "syscalls.h"
#include "direct_printf.h"

void handle_noop(PipeOperation *op) {
    (void)op;
    reset_current_client_pid();
}

void send_error_msg(int32_t operation) {
    give_response(operation, ERROR, 0, 0, 0, 0);
}

void handle_open(PipeOperation *op) {
    int32_t app_pid = op->app_id;
    int32_t fd[2];
    //direct_printf("PIPE OPEN [%d]\n", op->app_id);
    int32_t res = pipe_open(app_pid, fd);
    //direct_printf("PIPE OPEN RES [%d] fd read: %d write: %d\n", res, fd[0], fd[1]);
    
    give_response(op->type_op, res, (uint32_t)fd, sizeof(fd), fd[0], fd[1]);
}

void handle_read(PipeOperation *op) {
    int32_t app_pid = op->app_id;
    int fd = op->fd;
    int count = op->len_content;
    //direct_printf("PIPE READ [%d]\n", op->app_id);
    char buffer[MAX_BUFFER_IPC_SIZE]; 
    int to_read = (count > (int)sizeof(buffer)) ? (int)sizeof(buffer) : count;

    int32_t bytes_read = pipe_read(fd, app_pid, buffer, to_read);
    
    int len_to_send = (bytes_read > 0) ? bytes_read : 0;

    give_response(PIPE_OP_READ, bytes_read, (uint32_t)buffer, len_to_send, 0, 0);
}

void handle_write(PipeOperation *op) {
    int32_t app_pid = op->app_id;
    int fd = op->fd;
    int count = op->len_content;
    //direct_printf("PIPE WRITE [%d]\n", op->app_id);
    char buffer[MAX_BUFFER_IPC_SIZE];
    int to_write = (count > (int)sizeof(buffer)) ? (int)sizeof(buffer) : count;

    virtual_copy(app_pid, op->content_vaddr, (uint32_t)buffer, to_write);

    int bytes_written = pipe_write(fd, app_pid, buffer, to_write);

    give_response(PIPE_OP_WRITE, bytes_written, 0, 0, 0, 0);
}

void handle_close(PipeOperation *op) {
    int32_t app_pid = op->app_id;
    int fd = op->fd;
    //direct_printf("PIPE CLOSE [%d] fd [%d]\n", op->app_id, fd);
    int32_t res = pipe_close(fd, app_pid);

    give_response(PIPE_OP_CLOSE, res, 0, 0, fd, 0);
}

void handle_dup(PipeOperation *op) {
    int32_t app_pid = op->app_id;
    int32_t fd = op->fd;
    //direct_printf("PIPE DUP [%d] fd %d trg: %d\n", op->app_id, fd, op->arg_1);
    int32_t res = pipe_dup(fd, app_pid);
    
    give_response(PIPE_OP_DUP, res, 0, 0, fd, op->arg_1);
}


void handle_fork(PipeOperation *op) {
    int32_t app_pid = op->app_id;
    int32_t app_father = op->arg_1;
    //direct_printf("PIPE FORK [%d]\n", op->app_id);
    int32_t res = pipe_fork(app_pid, app_father);
    
    give_response(PIPE_OP_FORK, res, 0, 0, 0, 0);
}

void handle_close_all(PipeOperation *op) {
    int32_t app_pid = op->app_id;
    //direct_printf("PIPE CLOSE ALL [%d]\n", op->app_id);
    int32_t res = pipe_close_all(app_pid);
    
    give_response(PIPE_OP_CLOSE_ALL, res, 0, 0, 0, 0);
}

void handle_fstat(PipeOperation *op) {
    PANIC("unimplemented yet");
}

typedef void (*pipe_op_handler_t)(PipeOperation *op);

static const pipe_op_handler_t op_dispatch_table[] = {
    [PIPE_OP_PING]      = handle_noop,
    [PIPE_OP_OPEN]      = handle_open,
    [PIPE_OP_READ]      = handle_read,
    [PIPE_OP_WRITE]     = handle_write,
    [PIPE_OP_CLOSE]     = handle_close,
    [PIPE_OP_FSTAT]     = handle_fstat,
    [PIPE_OP_DUP]       = handle_dup,
    [PIPE_OP_CLOSE_ALL] = handle_close_all,
    [PIPE_OP_FORK]      = handle_fork
};

#define MAX_OP_HANDLERS (sizeof(op_dispatch_table) / sizeof(op_dispatch_table[0]))

void dispatch_command(PipeOperation *op) {
    if (op->type_op >= 0 && (size_t)op->type_op < MAX_OP_HANDLERS && op_dispatch_table[op->type_op]) {
        op_dispatch_table[op->type_op](op);
    } else {
        send_error_msg(op->type_op);
    }
}

void exec_command(PipeOperation command) {
    dispatch_command(&command);
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    // enable_debug_print(); // Descomentar si es necesario para salida serial/consola
    pipe_init();
    
    PipeOperation command;

    while(1){
        if (get_command(&command) == SUCCESS) {
            exec_command(command);
        }
    }
    
    return 0;
}