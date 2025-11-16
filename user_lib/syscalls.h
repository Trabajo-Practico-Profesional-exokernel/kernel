#ifndef SYSCALLS_H
#define SYSCALLS_H

void putchar(char ch);

int getchar(void);


// Receives index of program to exec and pointer to the args to exec with.
int exec(int prog_ind, char ** args);

#endif