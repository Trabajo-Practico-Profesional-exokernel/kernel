/* block.h */

#ifndef BLOCK_INCLUDED
#define BLOCK_INCLUDED

#include "common.h"

#define BLOCK_SIZE_BITS 9
#define BLOCK_SIZE (1 << BLOCK_SIZE_BITS)
#define BLOCK_MASK (BLOCK_SIZE-1)


void bzero_block(char *block);

void block_init(void);

int block_read(int block, char *mem);


int block_write(int block, char *mem);

#endif