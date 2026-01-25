#pragma once
#ifndef BASIC_LIB_FUNCTIONS
#define BASIC_LIB_FUNCTIONS
#include "stdio.h"

// Include these to save time on user programs
#include "types.h"
#include "syscalls.h"


void sleep(int delay);

int syscall(int sysno, int arg0, int arg1, int arg2);


// General functionality defined in user_lib/*.c
int get_string(char *buf, int max_len);

#endif