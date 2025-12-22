#include "inc/syscalls.h"
#include "syscalls.h" // Def of syscalls implemented here.
#include "lib.h" // For printf and syscall func

int exec(int prog_ind, char ** args){
    // convert args pointer to int
    return syscall(SYS_EXEC, prog_ind, (int)(args), 0);
}


void sys_send_msg(int proc_pid, char *msg, size_t len_msg) {
    printf("to %d msg '%s' len? %u\n",proc_pid, *msg, len_msg);

    // syscall(SYS_SEND_MSG, proc_pid, (int)(msg), len_msg);
}

void sendchar(int proc_pid, char ch) {
    printf("sending char\n");
    syscall(SYS_SENDCHAR, proc_pid, ch, 0);
}

void sendbyte(int proc_pid, char ch) {
    syscall(SYS_SEND_BYTE, proc_pid, ch, 0);
}

char recvbyte() {
    return syscall(SYS_RECV_BYTE, 0, 0, 0);
}

char recv_msg() {
    return (char)syscall(SYS_RECV_MSG, 0, 0, 0); 
}

void recvchar(int proc_pid, char ch) {
    printf("receiving char\n");
    syscall(SYS_RECVCHAR, proc_pid, ch, 0);
}

void putchar(char ch) {
    syscall(SYS_PUTCHAR, ch, 0, 0);
}

int getchar(void) {
    return syscall(SYS_GETCHAR, 0, 0, 0);
}


int wait(int pid){
    return syscall(SYS_WAIT, pid, 0, 0);
}

void sys_yield(){
    syscall(SYS_YIELD, 0, 0, 0);
}

__attribute__((noreturn)) void exit(int ret_code) {
    syscall(SYS_EXIT, ret_code, 0, 0);
    // SHOULD NEVER HAPPEN... just to make compiler shutup
    printf("SHOULD NOT REACH HERE! AFTER EXIT\n");
    for(;;){

    }
}




///
/// FILESYSTEM
///

int register_fs_handler(struct FilesystemEventsHandler* handler){
    return syscall(SYS_FS_REG_HANDLER, (int) handler, 0 , 0);
}

void sys_fs_ret(int ret_code){
    syscall(SYS_FS_RET, ret_code, 0 , 0);
}



int sys_touch(char* filepath){
    return syscall(SYS_FS_TOUCH, (int) filepath, 0, 0);
}

int sys_rm(char* filepath){
    return syscall(SYS_FS_RM, (int) filepath, 0, 0);
}

int sys_stat(char* filepath){
    return syscall(SYS_FS_STAT, (int) filepath, 0, 0);
}

int getpid(void) {
    return syscall(SYS_GETPID, 0, 0, 0);
}

int uptime(void) {
    return syscall(SYS_UPTIME, 0, 0, 0);
}