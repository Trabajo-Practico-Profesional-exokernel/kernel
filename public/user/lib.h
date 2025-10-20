#pragma once
#include "inc/common.h"

__attribute__((noreturn)) void exit(void);

void putchar(char ch);

int getchar(void);

void sleep(int delay);

int syscall(int sysno, int arg0, int arg1, int arg2);