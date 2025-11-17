#ifndef SYSCALLS_H
#define SYSCALLS_H

void putchar(char ch);

int getchar(void);


// Receives index of program to exec and pointer to the args to exec with.
int exec(int prog_ind, char ** args);

void sys_yield(void);

int wait(int pid);
////
//// Calls syscall exit or so.
////
__attribute__((noreturn)) void exit(int ret_code);




#endif