#pragma once
#ifndef BASIC_LIB_FUNCTIONS
#define BASIC_LIB_FUNCTIONS
#include "stdio.h"

// Include these to save time on user programs
#include "types.h"
#include "syscalls.h"


void sleep(int delay);

int syscall(int sysno, int arg0, int arg1, int arg2, int arg3);


// General functionality defined in user_lib/*.c
int get_string(char *buf, int max_len);

// Memory allocation functions (malloc.c)
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *calloc(size_t count, size_t size);
void malloc_stats(void);  // Debug function to print heap statistics

#endif