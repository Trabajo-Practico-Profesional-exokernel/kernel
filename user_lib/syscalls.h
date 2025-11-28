#ifndef SYSCALLS_H
#define SYSCALLS_H

void putchar(char ch);

int getchar(void);


// Receives index of program to exec and pointer to the args to exec with.
int exec(int prog_ind, char ** args);
void send_msg(int pid_proc, char * msg_out, int len_msg_out);
void sendchar(int proc_pid, char ch);
void sys_yield(void);
char recv_msg();
char recvbyte();
void sendbyte(int proc_pid, char ch);
int wait(int pid);
////
//// Calls syscall exit or so.
////
__attribute__((noreturn)) void exit(int ret_code);




#endif