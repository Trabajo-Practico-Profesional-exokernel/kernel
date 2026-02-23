#include "inc/syscalls.h"
#include "syscalls.h"
#include "lib.h"
#include "inc/filesystem.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "console/debug.h"
#include "ipc.h"
#include "ipc_msgs.h"
#include "inc/operations.h"
#include "constants.h"
#include "parsers/strutil.h"
#include "arch/console.h"
#include "arch/proc.h"

int sys_exec(char ** args){
    return syscall(SYS_EXEC, (int)(args),0, 0, 0);
}


int sys_fork(void){

    int pid = syscall(SYS_FORK, 0, 0, 0, 0);

    if (pid == 0) {
        int res = app_recv_fork_msg();
        if (res == ERROR){
            printf("error en fork hijo\n");
            return ERROR;
        }
        printf("devolviendo pid [%d]\n", pid);
    } else if (pid > 0) {
        int res = app_send_fork_msg(pid);
        if (res == ERROR){
            return ERROR;
        }
    }
    return pid;
}

int sys_sleep(int time){
    return syscall(SYS_SLEEP, time, 0, 0, 0);
}


int wait(int pid){
    return syscall(SYS_WAIT, pid, 0, 0, 0);
}


int sys_kill(int pid){
    return syscall(SYS_KILL, pid, 0, 0, 0);
}

void sys_yield(){
    syscall(SYS_YIELD, 0, 0, 0, 0);
}

__attribute__((noreturn)) void exit(int ret_code) {
    app_send_close_msg();
    syscall(SYS_EXIT, ret_code, 0, 0, 0);
    printf("SHOULD NOT REACH HERE! AFTER EXIT\n");
    for(;;){}
}

int getpid(void) {
    return syscall(SYS_GETPID, 0, 0, 0, 0);
}

void * sbrk(const int count_pages){
    return (void *) syscall(SYS_SBRK, count_pages, 0, 0, 0);
}

int uptime(void) {
    return syscall(SYS_UPTIME, 0, 0, 0, 0);
}

int alive(int pid){
    return syscall(SYS_ALIVE, pid, 0, 0, 0);
}

int get_coord_pid(){
    return syscall(SYS_COORDPID, 0, 0, 0, 0);
}

int console_read(char *buf, int len){
    return syscall(SYS_CONSOLE_GET, (int)buf, len, 0, 0);
}

int console_write(char *buf, int len){
    return syscall(SYS_CONSOLE_PUT, (int)buf, len, 0, 0);
}

int console_close(int fd){
    return syscall(SYS_CONSOLE_CLOSE, fd, 0, 0, 0);
}

int procls(){
    return syscall(SYS_PROC_LS, 0, 0, 0, 0);
}

void putchar(char ch) {
    write(STDOUT, &ch, 1);
}

int getchar(void) {
    char c;
    int bytes = read(STDIN, &c, 1);
    if (bytes > 0) return c;
    return ERROR;
}

int try_send_content(int proc_pid, char *content, int len_content){
    return syscall(SYS_TRY_SEND_CONTENT, proc_pid, (int)content, len_content, 0);
}

int try_recv_content(char *content, int len_content){
    return syscall(SYS_TRY_RECV_CONTENT, (int)content, len_content, 0, 0);
}

int recv_content(char *content, int len_content){
    return syscall(SYS_RECV_CONTENT, (int)content, len_content, 0, 0);
}

int virtual_copy(uint32_t pid_src_proc, uint32_t src_addr, uint32_t dst_addr, int len){
    return syscall(SYS_VIRTUAL_COPY, src_addr, dst_addr, len, pid_src_proc);
}

int open(const char *path, int mode) {
    int res = app_send_msg_to_server(OP_OPEN, 0, mode, 0, (int)path, strlen((const uint8_t *) path) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_OPEN);
}

int close(int fd) {
    int server_pid = get_server_pid(OP_CLOSE, fd, 0, 0, 0, 0);
    
    if (server_pid < 0) {
        if (fd == STDIN || fd == STDOUT) {
            int res = console_close(fd);
            send_msg(get_coord_pid(), OP_UPDATE, KERNEL, getpid(), fd, -1, 0);
            return (res > 0) ? res : 0;
        }
        return ERROR;
    }
    
    int real_fd = get_real_fd(OP_CLOSE, fd, server_pid);
    int target_fd = (real_fd >= 0) ? real_fd : fd;

    int res = app_send_msg_to_server(OP_CLOSE, target_fd, 0, 0, 0, 0);
    if (res == ERROR) return ERROR;
    
    return app_receive_parameter(OP_CLOSE);
}

int read(int fd, char *buf, int size) {
    if (fd == STDIN){
        int res = console_read(buf, size);
        if (res > 0) return res;
    }
    
    while (1) {
        int res = app_send_msg_to_server(OP_READ, fd, 0, 0, (int)buf, size);
        if (res == ERROR) return ERROR;
        
        res = app_receive_content(OP_READ, buf, size);
        
        if (res != -2) {
            return res;
        }
        
        sys_yield();
    }
}

int write(int fd, char *content, int len) {

    if (fd == STDOUT){
        int res = console_write(content, len);
        if (res > 0) return res;
    }

    int res = app_send_msg_to_server(OP_WRITE, fd, 0, 0, (int)content, len);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_WRITE);
}

int lseek(int fd, int offset, int whence) {
    (void)whence;
    //VERIFICAR CORRECTAMENTE EL ENVIO Y RECIBO DE MENSAJES CUANDO LSEEK FALLA
    int res = app_send_msg_to_server(OP_LSEEK, fd, offset, 0, 0, 0);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_LSEEK);
}

int stat(char* filepath){
    int res = app_send_msg_to_server(OP_STAT, 0, 0, 0, (int)filepath, strlen((const uint8_t *) filepath) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_STAT);
}

int mkdir(const char *path) {
    int res = app_send_msg_to_server(OP_MKDIR, 0, 0, 0, (int)path, strlen((const uint8_t *) path) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_MKDIR);
}

int rmdir(const char *path) {
    int res = app_send_msg_to_server(OP_RMDIR, 0, 0, 0, (int)path, strlen((const uint8_t *) path) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_RMDIR);
}

int chdir(const char *path) {
    int res = app_send_msg_to_server(OP_CHDIR, 0, 0, 0, (int)path, strlen((const uint8_t *) path) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_CHDIR);
}

int getcwd(char *buf, int size) {
    int res = app_send_msg_to_server(OP_CWD, 0, 0, 0, (int)buf, size);
    if (res == ERROR) return ERROR;
    return app_receive_content(OP_CWD, buf, size);
}

int ls(const char *path) {
    int res = app_send_msg_to_server(OP_LS, 0, 0, 0, (int)path, strlen((const uint8_t *) path) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_LS);
}

int mknod(const char *path, short major, short minor) {
    (void)major; (void)minor;
    int res = app_send_msg_to_server(OP_MKNOD, 0, 0, 0, (int)path, strlen((const uint8_t *) path) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_MKNOD);
}

int link(const char *old_path, const char *new_path) {
    int len_old = strlen((const uint8_t *) old_path) + 1;
    int len_new = strlen((const uint8_t *) new_path) + 1;
    
    int res = app_send_msg_to_server(OP_LINK, (int)old_path, len_old, (int)new_path, len_new, 0);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_LINK);
}

int unlink(const char *path) {
    int res = app_send_msg_to_server(OP_UNLINK, 0, 0, 0, (int)path, strlen((const uint8_t *) path) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_UNLINK);
}

int chown(const char *pathname, uint32_t owner, uint32_t group){
    int res = app_send_msg_to_server(OP_CHOWN, 0, owner, group, (int)pathname, strlen((const uint8_t *) pathname) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_CHOWN);
}

int chmod(const char *pathname, uint32_t mode){
    int res = app_send_msg_to_server(OP_CHMOD, 0, mode, 0, (int)pathname, strlen((const uint8_t *) pathname) + 1);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_CHMOD);
}

int pipe(int fds[2]){
    int res = app_send_msg_to_server(OP_PIPE, 0, 0, 0, (int)fds, sizeof(int)*2);
    if (res == ERROR) return ERROR;
    return app_receive_content(OP_PIPE, fds, sizeof(int)*2);
}

int dup(int prev_fd){
    int res = app_send_msg_to_server(OP_DUP, prev_fd, 0, 0, 0, 0);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_DUP);
}

int dup2(int target_fd, int prev_fd) {
    if (prev_fd == target_fd) {
        return target_fd;
    }

    if (close(prev_fd) == ERROR){
        return ERROR;
    }

    if (dup(target_fd) == ERROR){
        return ERROR;
    }
    
    return SUCCESS;
}

int sys_execv(char* new_prog_name, char ** argv){
    return syscall(SYS_PROC_EXECV,(int)(new_prog_name),(int)(argv), 0, 0);
}

