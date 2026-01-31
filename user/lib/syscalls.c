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

int exec(int prog_ind, char ** args){
    return syscall(SYS_EXEC, prog_ind, (int)(args), 0, 0);
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

void putchar(char ch) {
    syscall(SYS_PUTCHAR, ch, 0, 0, 0);
}

int getchar(void) {
    return syscall(SYS_GETCHAR, 0, 0, 0, 0);
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
    int res = app_send_msg_to_server(OP_CLOSE, fd, 0, 0, 0, 0);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_CLOSE);
}

int read(int fd, char *buf, int size) {
    int res = app_send_msg_to_server(OP_READ, fd, 0, 0, (int)buf, size);
    if (res == ERROR) return ERROR;
    return app_receive_content(OP_READ, buf, size);
}

int write(int fd, char *content, int len) {
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
    PANIC("stat not implemented yet");
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
    return app_receive_parameter(OP_PIPE);
}

int dup(int prev_fd){
    int res = app_send_msg_to_server(OP_DUP, prev_fd, 0, 0, 0, 0);
    if (res == ERROR) return ERROR;
    return app_receive_parameter(OP_DUP);
}
