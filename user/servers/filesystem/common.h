#ifndef COMMON_H
#define COMMON_H

#define BLOCK_SIZE 512
#define FILESYSTEM_TOTAL_BLOCKS 64
#define MAGIC_NUMBER 0x42
#define MAX_PATH_NAME 256 
#define SUPER_BLOCK_POSITION 0
#define IMAP_POSITION 1
#define DMAP_POSITION 2
#define FIRST_INODE_POSITION 3
#define FIRST_DATA_BLOCK_POSITION 8

#define INODE_BLOCKS 5
#define DATA_REGION_BLOCKS 56   // 64 - 8 = 56
#define INODE_SIZE 32           // Bytes

#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE) // 512 / 32 = 16 inodos por bloque
#define DIRECT_POINTERS 6       // Ajustado para encajar en 32 bytes


#endif