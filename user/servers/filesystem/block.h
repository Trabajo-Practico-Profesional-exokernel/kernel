#ifndef BLOCK_INCLUDED
#define BLOCK_INCLUDED

#include "disk_syscalls.h"

#define BLOCK_SIZE 512

int block_read( int block, char *mem);
int block_write(int block, char *mem);
int set_zero_block(int block_index);


#endif
