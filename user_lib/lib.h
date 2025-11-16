#pragma once
#ifndef BASIC_LIB_FUNCTIONS
#define BASIC_LIB_FUNCTIONS
#include "inc/common.h"

// Include these to save time on user programs
#include "inc/types.h"
#include "inc/syscalls.h"

////
//// Defined by arch/user/entry_point.c
////
__attribute__((noreturn)) void exit(void);

void putchar(char ch);

int getchar(void);

void sleep(int delay);

int syscall(int sysno, int arg0, int arg1, int arg2);


// General functionality defined in user_lib/*.c
int get_string(char *buf, int max_len);

#endif