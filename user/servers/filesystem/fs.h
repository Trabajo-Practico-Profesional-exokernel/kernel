/* fs.h */

#ifndef FS_INCLUDED
#define FS_INCLUDED

#include "common.h"
#include "block.h"

#define FS_SIZE 2048
#define MAX_FILE_NAME 28
#define MAX_PATH_NAME 256  
#define MAX_OPEN_FILES 256
#define MAGIC_NUMBER 0x45

#define INODES_BLOCKS 256
#define INODES_PER_BLOCK 8 // Must be less than or equal to 8
#define DIRECT_POINTERS 10 
#define POINTERS_PER_DCB 12

// Definiciones de cálculo de espacio y mapas
#define INODES_NUMBER (INODES_BLOCKS * INODES_PER_BLOCK)
#define DATA_BLOCKS (FS_SIZE - 2 - INODES_BLOCKS)
#define IMAP_BYTES ((INODES_NUMBER+7)/8)
#define DMAP_BYTES ((DATA_BLOCKS+7)/8)
#define MAP_BLOCK (1 + INODES_BLOCKS)

#define LOG2NPROC 4
#define NPROC (1 << LOG2NPROC)
#define PROCS_MAX NPROC        
#define PROCX(procid) ((procid) & (PROCS_MAX - 1))

// Superblock
typedef struct{
    uint32_t size_disk;
    uint32_t block_size;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint32_t num_blocks_inodes;

    uint32_t beg_inodes;
    uint32_t beg_map;
    uint32_t beg_data;

    uint32_t pointers_per_block;
    uint32_t pointers_per_dcb;
    uint32_t inodes_per_block;
    uint32_t direct_pointers;
    
    uint32_t magic_number;
} superblock_t;

// Inode
typedef struct{
    int type;
    int link_counter;
    int size;
    int direct[DIRECT_POINTERS];
    int indirect1;
    int indirect2;
    int indirect3;
} inode_t;

// Definimos una estructura temporal para guardar los datos en memoria
typedef struct {
    char name[32]; // Ajusta al tamaño máximo de nombre en tu FS
    int inum;
    int size;
    char type;
} file_info_t;

// Bitmaps
typedef struct{
    char imap[IMAP_BYTES];
    char dmap[DMAP_BYTES];
} bmap_t;

// Directory structure
typedef struct{
    uint8_t files_name[POINTERS_PER_DCB][MAX_FILE_NAME];
    int files_inum[POINTERS_PER_DCB];
} dir_t;

// DataBlock union
typedef union{
    dir_t dir;
    int pointers[BLOCK_SIZE/4];
    int8_t data[BLOCK_SIZE];
} DataBlock;

// Generic Block union
typedef union{
    superblock_t sb;
    inode_t inodes[INODES_PER_BLOCK];
    bmap_t map;
    DataBlock data_block;
} Block;

// Filesystem Check struct
typedef struct{
    uint32_t magic_number;
    int blocks_allocated;
    int inodes_allocated;
    bmap_t map;
} fsCheck;

// Open-files table entry
typedef struct{
    int fd;
    char name[MAX_PATH_NAME];
    int inode;
    int flag;
    int rw_ptr;
} FileDescriptor; 

// Inicialización y formateo
void fs_init(void);
int fs_mkfs(void);

// Funciones principales del FS (Internal Logic)
// Estas funciones serán invocadas por el dispatcher de IPC
int fs_open(char *fileName, int flags, int proc);
int fs_close(int fd, int proc);
int fs_read(int fd, char *buf, int count, int proc);
int fs_write(int fd, char *buf, int count, int proc);
int fs_lseek(int fd, int offset, int proc);
int fs_mkdir(char *fileName, int proc);
int fs_rmdir(char *fileName, int proc);
int fs_cd(char *dirName, int proc);
int fs_link(char *old_fileName, char *new_fileName, int proc);
int fs_unlink(char *fileName, int proc);
int fs_stat(char *fileName, fileStat *buf, int proc);
int fs_fsck(fsCheck *buf);

void fs_ls_buffered(int proc);
void shell_ls(int proc);
void fs_sync_current_dir(int proc);
void fs_reload_current_dir(int proc);
void fs_pwd(int proc);
#endif