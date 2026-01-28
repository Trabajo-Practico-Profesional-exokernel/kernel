#include "inc/syscalls.h"
#include "syscalls.h"
#include "lib.h"
#include "arch/communication.h"
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

int virtual_copy(uint32_t src_addr, uint32_t dst_addr, int len){
    return syscall(SYS_VIRTUAL_COPY, src_addr, dst_addr, len, 0);
}

int open(const char *path, int mode) {
    (void)mode;
    int res = send_msg_to_service(OP_OPEN, 0, 0, 0, (void*)path, strlen((const uint8_t*)path));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_OPEN);
}

int close(int fd) {
    int res = send_msg_to_service(OP_CLOSE, fd, 0, 0, NULL, 0);
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_CLOSE);
}

int read(int fd, char *buf, int size) {
    int res = send_msg_to_service(OP_READ, fd, 0, 0, buf, size);
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_READ);
}

int write(int fd, char *content, int len) {
    int res = send_msg_to_service(OP_WRITE, fd, 0, 0, content, len);
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_WRITE);
}

int lseek(int fd, int offset, int whence) {
    (void)whence;
    int res = send_msg_to_service(OP_LSEEK, fd, offset, 0, NULL, 0);
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_LSEEK);
}

int stat(char* filepath){
    int res = send_msg_to_service(OP_STAT, 0, 0, 0, filepath, strlen((const uint8_t*)filepath));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_STAT);
}

int mkdir(const char *path) {
    int res = send_msg_to_service(OP_MKDIR, 0, 0, 0, (void*)path, strlen((const uint8_t*)path));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_MKDIR);
}

int rmdir(const char *path) {
    int res = send_msg_to_service(OP_RMDIR, 0, 0, 0, (void*)path, strlen((const uint8_t*)path));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_RMDIR);
}

int chdir(const char *path) {
    int res = send_msg_to_service(OP_CHDIR, 0, 0, 0, (void*)path, strlen((const uint8_t*)path));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_CHDIR);
}

int getcwd(char *buf, int size) {
    int res = send_msg_to_service(OP_CWD, 0, 0, 0, buf, size);
    if (res == ERROR) return ERROR;
    return app_receive_content(OP_CWD, buf, size);
}

int ls(const char *path) {
    int res = send_msg_to_service(OP_LS, 0, 0, 0, (void*)path, strlen((const uint8_t*)path));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_LS);
}

int mknod(const char *path, short major, short minor) {
    (void)major; (void)minor;
    int res = send_msg_to_service(OP_MKNOD, 0, 0, 0, (void*)path, strlen((const uint8_t*)path));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_MKNOD);
}

int link(const char *old_path, const char *new_path) {
    char buf[MAX_CONTENT_SIZE];
    join_strings(buf, old_path, new_path);
    int res = send_msg_to_service(OP_MKDIR, 0, 0, 0, buf, strlen((const uint8_t*)buf));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_MKDIR);
}

int unlink(const char *path) {
    int res = send_msg_to_service(OP_UNLINK, 0, 0, 0, (void*)path, strlen((const uint8_t*)path));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_UNLINK);
}

int chown(const char *pathname, uint32_t owner, uint32_t group){
    int res = send_msg_to_service(OP_CHOWN, owner, group, 0, (void*)pathname, strlen((const uint8_t*)pathname));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_CHOWN);
}

int chmod(const char *pathname, uint32_t mode){
    int res = send_msg_to_service(OP_CHMOD, mode, 0, 0, (void*)pathname, strlen((const uint8_t*)pathname));
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_CHMOD);
}

int pipe(int fds[2]){
    int res = send_msg_to_service(OP_PIPE, fds[0], fds[1], 0, NULL, 0);
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_PIPE);
}

int dup(int prev_fd){
    int res = send_msg_to_service(OP_DUP, prev_fd, 0, 0, NULL, 0);
    if (res == ERROR) return ERROR;
    return app_receive_ok_msg(OP_DUP);
}
