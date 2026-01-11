/* main.c */
#include "lib.h"
#include "inc/filesystem.h"
#include "inc/syscalls.h"
#include "arch/communication.h"
#include "std/string.h"
#include "disk_syscalls.h"

#include "fs.h"
#include "util.h"
#include "fsUtil.h"
#include "block.h"
#include "common.h"
#include "inc/filesystem.h"

extern char current_path[PROCS_MAX][MAX_PATH_NAME];

void send_int_response(int pid_target, int value) {
    sys_try_send_msg(pid_target, (char *)&value, 1);
}

void send_data_response(int pid_target, void *data, int size) {
    sys_try_send_msg(pid_target, (char *)data, 1);
}

void handle_open(struct Message *msg) {
    int fd = fs_open((char *)msg->content, FS_O_RDWR, msg->sender_pid);
    send_int_response(msg->sender_pid, fd);
}

void handle_close(struct Message *msg) {
    int fd = *(int *)msg->content;
    int res = fs_close(fd, msg->sender_pid);
    send_int_response(msg->sender_pid, res);
}

void handle_read(struct Message *msg) {
    fs_rw_req_t *req = (fs_rw_req_t *)msg->content;
    char buffer[1024];
    // Limitamos la lectura al tamaño del buffer de stack o al solicitado
    int to_read = (req->count > sizeof(buffer)) ? sizeof(buffer) : req->count;
    
    int bytes_read = fs_read(req->fd, buffer, to_read, msg->sender_pid);

    if (bytes_read < 0) {
        send_data_response(msg->sender_pid, NULL, 0);
    } else {
        send_data_response(msg->sender_pid, buffer, bytes_read);
    }
}

void handle_write(struct Message *msg) {
    fs_rw_req_t *req = (fs_rw_req_t *)msg->content;

    int valid_len = msg->content_size - sizeof(fs_rw_req_t);
    if (req->count > valid_len) {
        req->count = valid_len;
    }

    int bytes_written = fs_write(req->fd, req->data, req->count, msg->sender_pid);
    send_int_response(msg->sender_pid, bytes_written);
}

void handle_mkdir(struct Message *msg) {
    int result = fs_mkdir((char *)msg->content, msg->sender_pid);
    if (result == 0) {
        fs_sync_current_dir(msg->sender_pid); // Sincronizar con disco
    }
    //shell_ls(msg->sender_pid);
    send_int_response(msg->sender_pid, result);
}

void handle_rmdir(struct Message *msg) {
    //printf("REMOVING DIR\n");
    int result = fs_rmdir((char *)msg->content, msg->sender_pid);
    if (result == 0) {
        fs_sync_current_dir(msg->sender_pid); // Sincronizar con disco
    }
    //printf("RESULT: [%d]\n", result);
    //shell_ls(msg->sender_pid);
    send_int_response(msg->sender_pid, result);
}

void handle_rm(struct Message *msg) {
    int result = fs_unlink((char *)msg->content, msg->sender_pid);

    send_int_response(msg->sender_pid, result);
}

void handle_fstat(struct Message *msg) {
    fileStat stat_buf;
    int res = fs_stat((char *)msg->content, &stat_buf, msg->sender_pid);
    if (res == 0) {
        send_data_response(msg->sender_pid, &stat_buf, sizeof(fileStat));
    } else {
        int error = -1;
        send_data_response(msg->sender_pid, &error, sizeof(int));
    }
}

void handle_link(struct Message *msg) {
    // Link recibe dos strings pegados. Hay que tener cuidado con strlen aquí.
    char *old = (char *)msg->content;
    // Buscamos el null terminator del primer string para encontrar el segundo
    int old_len = strlen(old);
    char *new = (char *)msg->content + old_len + 1;

    int res = fs_link(old, new, msg->sender_pid);
    send_int_response(msg->sender_pid, res);
}

void handle_chdir(struct Message *msg) {
    int res = fs_cd((char *)msg->content, msg->sender_pid);
    send_int_response(msg->sender_pid, res);
}

void handle_lseek(struct Message *msg) {
    fs_seek_req_t *req = (fs_seek_req_t *)msg->content;
    int res = fs_lseek(req->fd, req->offset, msg->sender_pid);
    send_int_response(msg->sender_pid, res);
}

void handle_mknod(struct Message *msg) {
    // Simulamos mknod abriendo el archivo para crearlo y cerrandolo inmediatamente
    int fd = fs_open((char *)msg->content, FS_O_RDWR, msg->sender_pid);
    if (fd >= 0) {
        fs_close(fd, msg->sender_pid);
        send_int_response(msg->sender_pid, 0);
    } else {
        send_int_response(msg->sender_pid, -1);
    }
}

void handle_shell_ls(struct Message *msg){
    shell_ls(msg->sender_pid);
    send_int_response(msg->sender_pid, 0);
}

void handle_pwd(struct Message *msg) {
    char *path = current_path[msg->sender_pid];
    send_data_response(msg->sender_pid, path, strlen(path) + 1);
}

#define MAX_HANDLERS (sizeof(dispatch_table) / sizeof(dispatch_table[0]))

typedef void (*fs_handler_t)(struct Message *);

static const fs_handler_t dispatch_table[] = {
    [FS_TYPE_OPEN] = handle_open,
    [FS_TYPE_CLOSE] = handle_close,
    [FS_TYPE_READ] = handle_read,
    [FS_TYPE_WRITE] = handle_write,
    [FS_TYPE_MKDIR] = handle_mkdir,
    [FS_TYPE_RMDIR] = handle_rmdir,
    [FS_TYPE_UNLINK] = handle_rm,
    [FS_TYPE_FSTAT] = handle_fstat,
    [FS_TYPE_LINK] = handle_link,
    [FS_TYPE_CHDIR] = handle_chdir,
    [FS_TYPE_LSEEK] = handle_lseek,
    [FS_TYPE_MKNOD] = handle_mknod,
    [FS_TYPE_LS] = handle_shell_ls,
    [FS_TYPE_PWD]= handle_pwd};

void dispatch_request(struct Message *msg) {
    if (msg->type >= 0 && msg->type < MAX_HANDLERS && dispatch_table[msg->type]) {
        dispatch_table[msg->type](msg);
    } else {
        debug_printf("[FS] Unknown msg type %d\n", msg->type);
        send_int_response(msg->sender_pid, -1);
    }
}

void server_listen() {
    struct Message msg;
    while (1) {
        int res = sys_recv_msg(&msg);
        if (res == 0) {
            debug_printf("mensaje recibido con contenido: [%d], [%s]", msg.type, msg.content);
            dispatch_request(&msg);
        }
    }
}

void main() {
    disable_debug_print();
    debug_printf("FILESYSTEM!");
    fs_init();
    server_listen();
}