#include "inc/syscalls.h"

#include "syscalls.h" // Def of syscalls implemented here.
#include "lib.h" // For printf and syscall func

int exec(int prog_ind, char ** args){
    // convert args pointer to int
    return syscall(SYS_EXEC, prog_ind, (int)(args), 0);
}


void send_msg(int proc_pid, char *msg, int len_msg) {
    printf("msg '%s'\n", *msg);

    syscall(SYS_SEND_MSG, proc_pid, (int)(msg), len_msg);
}

void sendchar(int proc_pid, char ch) {
    printf("sending char\n");
    syscall(SYS_SENDCHAR, proc_pid, ch, 0);
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
