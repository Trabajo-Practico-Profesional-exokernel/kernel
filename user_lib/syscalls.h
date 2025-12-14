#ifndef SYSCALLS_H
#define SYSCALLS_H


#include "inc/filesystem.h"

void putchar(char ch);

int getchar(void);


// Receives index of program to exec and pointer to the args to exec with.
int exec(int prog_ind, char ** args);

void sys_yield(void);
int wait(int pid);




//
// FILESYSTEM
//

// Registers the handler that will receive on the buffer the content of requests.
// It returns 0 If it suceeded, else If an error happened.
int register_fs_handler(struct FilesystemEventsHandler* handler);


int sys_touch(char* filepath);
int sys_rm(char* filepath);
int sys_stat(char* filepath);



void sys_send_msg(int pid_proc, char * msg_out, size_t len_msg_out);
// int recv_ipc_msg(char* msg, size_t max_len);
// void send_msg(int proc_pid, char* ch, size_t len);

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