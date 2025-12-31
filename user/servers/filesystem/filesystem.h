#ifndef FILESYSTEM_INCLUDED
#define FILESYSTEM_INCLUDED

#include "std/string.h"
#include "inc/types.h"
#include "block.h"
#define MAGIC_NUMBER 0xCAFEBEBE
#define FILESYSTEM_TOTAL_BLOCKS 64


// superblock
typedef struct{

	// metadata of the disk
	uint32_t size_disk; // 4 bytes
	uint32_t block_size; // 4 bytes
	uint32_t num_inodes; // 4 bytes
	uint32_t num_data_blocks; // 4 bytes
	uint32_t num_blocks_inodes; // 4 bytes
	
	uint32_t magic_number; // 4 byte
} superblock_t; // Total size = 36 bytes


typedef union{
	uint8_t data[BLOCK_SIZE];
} DataBlock; // Total size = 512 bytes

typedef union{
	superblock_t super_block;
	DataBlock data_block;
} Block;

void fs_init(void);
int fs_mkfs(void);

#endif