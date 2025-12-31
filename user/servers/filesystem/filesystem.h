#ifndef FILESYSTEM_INCLUDED
#define FILESYSTEM_INCLUDED

#include "inc/types.h"

#define BLOCK_SIZE 512
#define FILESYSTEM_TOTAL_BLOCKS 64
#define MAGIC_NUMBER 0xCAFEBEBE

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

typedef struct {
    uint32_t size_disk;         // Tamaño total en bytes (32768)
    uint32_t block_size;        // 512 bytes
    uint32_t num_inodes;        // 80 (5 bloques * 16 inodos)
    uint32_t num_data_blocks;   // 56
    uint32_t num_blocks_inodes; // 5
    uint32_t magic_number;      // Identificador del FS

    uint8_t padding[BLOCK_SIZE - 24]; 
} superblock_t;

typedef struct {
    uint16_t type;               // 2 bytes (0: Libre, 1: Archivo, 2: Directorio)
    uint16_t link_counter;       // 2 bytes
    uint32_t size;               // 4 bytes
    uint32_t direct[DIRECT_POINTERS]; // 24 bytes (6 punteros * 4 bytes)
} inode_t;


typedef struct {
    uint32_t inode;     // 4 bytes: Número de inodo
    char name[28];      // 28 bytes: Nombre del archivo
} dirent_t;


typedef struct {
    uint8_t data[BLOCK_SIZE];
} raw_block_t;


typedef union {
    raw_block_t raw;                     // Acceso byte a byte (para dmap/imap/data)
    superblock_t super_block;                  // Acceso como superbloque
    inode_t inodes[INODES_PER_BLOCK];    // Acceso como array de inodos (16 por bloque)
    dirent_t dirents[INODES_PER_BLOCK];  // Acceso como array de entradas de directorio (16 por bloque)
} Block;

extern superblock_t super;
void fs_init(void);
int fs_mkfs(void);

int fs_mkdir(char *filepath);



#endif