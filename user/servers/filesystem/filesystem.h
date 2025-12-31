#ifndef FILESYSTEM_INCLUDED
#define FILESYSTEM_INCLUDED

#include "std/string.h"

#define MAGIC_NUMBER 0x42
#define FILESYSTEM_SIZE 2048
// superblock
typedef struct{
	uint32_t magic_number; // 4 bytes
} superblock_t;

typedef union{
	superblock_t sb;
} Block;

void fs_init(void);
int fs_mkfs(void);

#endif