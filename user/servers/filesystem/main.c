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

extern dir_t current_dir;
extern superblock_t super;
// Estructuras auxiliares
typedef struct {
    int fd;
    int count;
    char data[0]; 
} fs_rw_req_t;

typedef struct {
    int fd;
    int offset;
} fs_seek_req_t;

void send_int_response(int pid_target, int value) {
    sys_try_send_msg(pid_target, (char*) &value, sizeof(int));
}

void send_data_response(int pid_target, void* data, int size) {
    sys_try_send_msg(pid_target, (char*) data, size);
}

void handle_open(struct Message *msg) {
    int fd = fs_open(msg->content, FS_O_RDWR);
    send_int_response(msg->sender_pid, fd);
}

void handle_close(struct Message *msg) {
    int fd = *(int*)msg->content;
    int res = fs_close(fd);
    send_int_response(msg->sender_pid, res);
}

void handle_read(struct Message *msg) {
    fs_rw_req_t *req = (fs_rw_req_t*) msg->content;
    char buffer[1024]; 
    int to_read = (req->count > sizeof(buffer)) ? sizeof(buffer) : req->count;
    int bytes_read = fs_read(req->fd, buffer, to_read);
    
    if (bytes_read < 0) {
        send_data_response(msg->sender_pid, NULL, 0); 
    } else {
        send_data_response(msg->sender_pid, buffer, bytes_read);
    }
}

void handle_write(struct Message *msg) {
    fs_rw_req_t *req = (fs_rw_req_t*) msg->content;
    int bytes_written = fs_write(req->fd, req->data, req->count);
    send_int_response(msg->sender_pid, bytes_written);
}

void handle_mkdir(struct Message *msg) {
    int result = fs_mkdir(msg->content);
    if (result == 0) {
        fs_sync_current_dir();  // Sincronizar con disco
    }
    shell_ls();
    msg->content[0] = result;
    send_int_response(msg->sender_pid, result);
}

void handle_rmdir(struct Message *msg) {
    printf("REMOVING DIR\n");
    int result = fs_rmdir(msg->content);
    if (result == 0) {
        fs_sync_current_dir();  // Sincronizar con disco
    }
    printf("RESULT: [%d]\n", result);
    shell_ls();
    msg->content[0] = result;
    send_int_response(msg->sender_pid, result);
}

void handle_rm(struct Message *msg) {
    int result = fs_unlink(msg->content);
    shell_ls();
    send_int_response(msg->sender_pid, result);
}

void handle_fstat(struct Message *msg) {
    fileStat stat_buf;
    int res = fs_stat(msg->content, &stat_buf);
    if (res == 0) {
        send_data_response(msg->sender_pid, &stat_buf, sizeof(fileStat));
    } else {
        int error = -1;
        send_data_response(msg->sender_pid, &error, sizeof(int));
    }
}

void handle_link(struct Message *msg) {
    // Link recibe dos strings pegados. Hay que tener cuidado con strlen aquí.
    char *old = msg->content;
    // Buscamos el null terminator del primer string para encontrar el segundo
    int old_len = strlen(old); 
    char *new = msg->content + old_len + 1;
    
    int res = fs_link(old, new);
    send_int_response(msg->sender_pid, res);
}

void handle_chdir(struct Message *msg) {
    int res = fs_cd(msg->content);
    send_int_response(msg->sender_pid, res);
}

/*void handle_lseek(struct Message *msg) {
    fs_seek_req_t *req = (fs_seek_req_t*) msg->content;
    int res = fs_lseek(req->fd, req->offset);
    send_int_response(msg->sender_pid, res);
}*/



void handle_mknod(struct Message *msg) {

}


void handle_lseek(struct Message *msg) {

}

#define MAX_HANDLERS (sizeof(dispatch_table) / sizeof(dispatch_table[0]))

typedef void (*fs_handler_t)(struct Message*);

static const fs_handler_t dispatch_table[] = {
    [FS_TYPE_OPEN]   = handle_open,
    [FS_TYPE_CLOSE]  = handle_close,
    [FS_TYPE_READ]   = handle_read,
    [FS_TYPE_WRITE]  = handle_write,
    [FS_TYPE_MKDIR]  = handle_mkdir,
    [FS_TYPE_RMDIR]  = handle_rmdir,
    [FS_TYPE_UNLINK] = handle_rm,
    [FS_TYPE_FSTAT]  = handle_fstat,
    [FS_TYPE_LINK]   = handle_link,
    [FS_TYPE_CHDIR]  = handle_chdir,
    [FS_TYPE_LSEEK] = handle_lseek,
    [FS_TYPE_MKNOD]  = handle_mknod
};

void dispatch_request(struct Message *msg) {
    if (msg->type >= 0 && msg->type < MAX_HANDLERS && dispatch_table[msg->type]) {
        dispatch_table[msg->type](msg);
    } else {
        printf("[FS] Unknown msg type %d\n", msg->type);
        send_int_response(msg->sender_pid, -1);
    }
}

void server_listen(){
    struct Message msg;
    while(1) {
        int res = sys_recv_msg(&msg); 
        if (res == 0) { 
            dispatch_request(&msg);
        }
    }
}

void main(){
    fs_init();
    server_listen();
}