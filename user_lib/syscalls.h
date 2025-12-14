#ifndef SYSCALLS_H
#define SYSCALLS_H

void putchar(char ch);

int getchar(void);


// Receives index of program to exec and pointer to the args to exec with.
int exec(int prog_ind, char ** args);
void send_msg(int pid_proc, char * msg_out, int len_msg_out);
void sendchar(int proc_pid, char ch);
void sys_yield(void);

int recv_fs_events(char* msg, int max_len);
// int recv_ipc_msg(char* msg, int max_len);
void sendbyte(int proc_pid, char ch);
int wait(int pid);
////
//// Calls syscall exit or so.
////
__attribute__((noreturn)) void exit(int ret_code);


// filesystem => recv_msg(char* msg, int max_len);
// proc_filesystem->buffer_recv = char* msg
// proc_filesystem->max_len = max_len

// proc_touch -> touch(char * nombre) 
// proc_cat -> cat(char * nombre) 

// int type = mkfile 
// char[] nombre 





#endif