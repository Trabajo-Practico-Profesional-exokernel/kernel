/* main.c */
#include "lib.h"
#include "inc/filesystem.h"
#include "inc/syscalls.h"

#include "string.h"
#include "stdlib.h"
#include "disk_syscalls.h"
#include "fs.h"
#include "fsUtil.h"
#include "block.h"
#include "common.h"
#include "inc/filesystem.h"
#include "stdio.h"
#include "console/debug.h"
#include "parsers/strutil.h"
#include "ipc.h"
#include "command_handler.h"
#include "inc/operations.h"
#include "constants.h"
#include "direct_printf.h"

extern char current_path[PROCS_MAX][MAX_PATH_NAME];

void handle_ping(FilesystemOperation *op) {
    // Lógica de ping/pong
    // RETORNAR RESPUESTA
}

void handle_open(FilesystemOperation *op) {
    //direct_printf("HANDLE OPEN\n");
    int32_t app_pid = op->app_id;
    char path[MAX_PATH_NAME];
    
    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)path, op->len_content_1);
    
    int flags = op->arg_1;

    int fd = fs_open(path, flags, app_pid);

    give_response(FS_OP_OPEN, fd, 0, 0, fd,-1);
}

void handle_close(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    int fd = op->fd;

    int res = fs_close(fd, app_pid);

    give_response(FS_OP_CLOSE, res, 0, 0, fd,-1);
}

void handle_read(FilesystemOperation *op) {
    //direct_printf("HANDLE READ\n");

    int32_t app_pid = op->app_id;
    int fd = op->fd;
    int count = op->len_content_1;

    char buffer[MAX_BUFFER_IPC_SIZE]; 
    int to_read = (count > sizeof(buffer)) ? sizeof(buffer) : count;

    int bytes_read = fs_read(fd, buffer, to_read, app_pid);
    
    give_response(FS_OP_READ, bytes_read, (uint32_t)buffer, strlen(buffer), 0,-1);
}

void handle_write(FilesystemOperation *op) {
    //direct_printf("HANDLE WRITE\n");
    int32_t app_pid = op->app_id;
    int fd = op->fd;
    int count = op->len_content_1;

    char buffer[1024];
    int to_write = (count > sizeof(buffer)) ? sizeof(buffer) : count;

    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)buffer, to_write);

    int bytes_written = fs_write(fd, buffer, to_write, app_pid);

    give_response(FS_OP_WRITE, bytes_written, 0, 0, 0,-1);
}

void handle_lseek(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    int fd = op->fd;
    int offset = op->arg_1;

    int res = fs_lseek(fd, offset, app_pid);

    give_response(FS_OP_LSEEK, res, 0, 0, 0,-1);
}

void print_file_stat(const char *filename, fileStat *st) {
    const char *type_str = (st->type == DIRECTORY) ? "directory" : "regular file";

    //direct_printf("  File: %s\n", filename);
    //direct_printf("  Size: %d | Blocks: %d\n", st->size, st->numBlocks);
    //direct_printf("  Ino: %d | Links: %d | Type: %s\n", st->inodeNo, st->links, type_str);
    
    //direct_printf("Access: %d%d%d\n", st->owner_perms, st->group_perms, st->other_perms);
}

void handle_fstat(FilesystemOperation *op) {
    //direct_printf("falta terminar el retorno correctamente");
    int32_t app_pid = op->app_id;
    
    char path[MAX_PATH_NAME];
    virtual_copy(app_pid, (uint32_t)op->content_1_vaddr, (uint32_t)path, op->len_content_1);
    
    fileStat stat_buf;
    int res = fs_stat(path, &stat_buf, app_pid);
    if (res == SUCCESS){
        print_file_stat(path, &stat_buf);
    }
    //give_response(FS_OP_FSTAT, res, &stat_buf, sizeof(fileStat), 0);
    give_response(FS_OP_FSTAT, res, 0, 0, 0,-1);
}

void handle_dup(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    int32_t fd = op->fd;

    int32_t res = fs_dup(fd, app_pid);
    
    give_response(FS_OP_DUP, res, 0, 0, fd, op->arg_1);
}

void handle_fork(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    int32_t app_father = op->arg_1;

    int32_t res = fs_fork(app_pid, app_father);
    
    give_response(FS_OP_FORK, res, 0, 0, 0,-1);
}

void handle_close_all(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;

    int32_t res = fs_close_all(app_pid);
    
    give_response(FS_OP_CLOSE_ALL, res, 0, 0, 0,-1);
}

void handle_mkdir(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    char path[MAX_PATH_NAME];

    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)path, op->len_content_1);

    int res = fs_mkdir(path, app_pid);
    if (res == 0) {
        fs_sync_current_dir(app_pid);
    }

    give_response(FS_OP_MKDIR, res, 0, 0, 0,-1);
}

void handle_rmdir(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    char path[MAX_PATH_NAME];

    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)path, op->len_content_1);

    int res = fs_rmdir(path, app_pid);
    if (res == 0) {
        fs_sync_current_dir(app_pid);
    }

    give_response(FS_OP_RMDIR, res, 0, 0, 0,-1);
}

void handle_chdir(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    char path[MAX_PATH_NAME];

    direct_printf("[HANDLE_CHDIR] Extrayendo PID de la operacion: %d\n", app_pid);

    direct_printf("[HANDLE_CHDIR] Iniciando copia virtual de path (App VADDR: 0x%x, Len: %d)\n", op->content_1_vaddr, op->len_content_1);
    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)path, op->len_content_1);

    direct_printf("[HANDLE_CHDIR] Copia virtual completada. Path extraido: %s\n", path);
    direct_printf("[HANDLE_CHDIR] Invocando fs_cd para PID %d con path '%s'\n", app_pid, path);

    int res = fs_cd(path, app_pid);

    direct_printf("[HANDLE_CHDIR] Retorno de fs_cd: %d. Emitiendo respuesta IPC.\n", res);
    give_response(FS_OP_CHDIR, res, 0, 0, 0,-1);
}

void handle_pwd(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    char *path = current_path[app_pid];
    
    if (path == NULL){
        give_response(FS_OP_PWD, ERROR, 0, 0, 0,-1);
    }

    give_response(FS_OP_PWD, SUCCESS, (uint32_t)path, strlen(path), 0,-1);
}

void handle_ls(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;

    shell_ls(app_pid);

    give_response(FS_OP_LS, SUCCESS, 0, 0, 0,-1);
}

void handle_mknod(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    char path[MAX_PATH_NAME];

    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)path, op->len_content_1);

    int fd = fs_open(path, FS_O_RDWR, app_pid);
    int res = -1;
    
    if (fd >= 0) {
        fs_close(fd, app_pid);
        res = 0;
    }

    give_response(FS_OP_MKNOD, res, 0, 0, 0,-1);
}

void handle_link(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    char old_path[MAX_PATH_NAME];
    char new_path[MAX_PATH_NAME];

    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)old_path, op->len_content_1);
    virtual_copy(app_pid, op->content_2_vaddr, (uint32_t)new_path, op->len_content_2);

    int res = fs_link(old_path, new_path, app_pid);

    give_response(FS_OP_LINK, res, 0, 0, 0,-1);
}

void handle_unlink(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    char path[MAX_PATH_NAME];

    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)path, op->len_content_1);

    int res = fs_unlink(path, app_pid);

    give_response(FS_OP_UNLINK, res, 0, 0, 0,-1);
}

void handle_chown(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    char path[MAX_PATH_NAME];
    
    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)path, op->len_content_1);
    
    int uid = op->arg_1;
    int gid = op->arg_2;

    int res = fs_chown(app_pid, path, uid, gid);

    give_response(FS_OP_CHOWN, res, 0, 0, 0,-1);
}

void handle_chmod(FilesystemOperation *op) {
    int32_t app_pid = op->app_id;
    char path[MAX_PATH_NAME];
    
    virtual_copy(app_pid, op->content_1_vaddr, (uint32_t)path, op->len_content_1);

    int mode = op->arg_1;

    int res = fs_chmod(app_pid, path, mode);

    give_response(FS_OP_CHMOD, res, 0, 0, 0,-1);
}

void send_error_msg(int32_t operation){
    give_response(operation, ERROR, 0, 0, 0,-1);
}

typedef void (*fs_op_handler_t)(FilesystemOperation *op);

static const fs_op_handler_t op_dispatch_table[] = {
    [FS_OP_PING]        = handle_ping,
    [FS_OP_OPEN]        = handle_open,
    [FS_OP_CLOSE]       = handle_close,
    [FS_OP_READ]        = handle_read,
    [FS_OP_WRITE]       = handle_write,
    [FS_OP_LSEEK]       = handle_lseek,
    [FS_OP_FSTAT]       = handle_fstat,
    [FS_OP_DUP]         = handle_dup,
    [FS_OP_MKDIR]       = handle_mkdir,
    [FS_OP_RMDIR]       = handle_rmdir,
    [FS_OP_CHDIR]       = handle_chdir,
    [FS_OP_PWD]         = handle_pwd,
    [FS_OP_LS]          = handle_ls,
    [FS_OP_MKNOD]       = handle_mknod,
    [FS_OP_LINK]        = handle_link,
    [FS_OP_UNLINK]      = handle_unlink,
    [FS_OP_CHOWN]       = handle_chown,
    [FS_OP_CHMOD]       = handle_chmod,
    [FS_OP_FORK]        = handle_fork,
    [FS_OP_CLOSE_ALL]   = handle_close_all,
};

#define MAX_OP_HANDLERS (sizeof(op_dispatch_table) / sizeof(op_dispatch_table[0]))

void dispatch_command(FilesystemOperation *op) {
    if (op->type_op >= 0 && op->type_op < MAX_OP_HANDLERS && op_dispatch_table[op->type_op]) {
        op_dispatch_table[op->type_op](op);
    } else {
        send_error_msg(op->type_op);
    }
}

void exec_command(FilesystemOperation command){
    dispatch_command(&command);
}

void main() {

    disable_debug_print();

    fs_init();

    FilesystemOperation command;

    while(1){
        get_command(&command);
        exec_command(command);
    }

}
