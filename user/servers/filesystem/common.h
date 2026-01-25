/* common.h */

#ifndef COMMON_H
#define COMMON_H

// Integración: Incluir definiciones de tipos y E/S del kernel
#include "types.h"
#include "stdio.h"


// Tamaño del sector
#define SECTOR_SIZE 512


// Tipos específicos usados por simple-linux-fs
typedef unsigned char uchar_t;
typedef unsigned int uint_t;
typedef unsigned long ulong_t;

#define FREE_INODE 0
#define DIRECTORY 1
#define FILE 2

#define FS_O_RDONLY 1
#define FS_O_WRONLY 2
#define FS_O_RDWR 3


#define I_R_OP 0x4
#define I_W_OP 0x2
#define I_X_OP 0x1

#define OWNER_PERMS(mode_bits) ((mode_bits >> 6) & 7)
#define GROUP_PERMS(mode_bits) ((mode_bits >> 3) & 7)
#define OTHERS_PERMS(mode_bits) (mode_bits & 7) 

#define MODE_MASK(mode_bits) ((mode_bits >> TYPE_SHIFT) & 7)

#define TYPE_SHIFT 9

#define BUILD_MODE(type, user, group, other) \
    ( (((type) & 7) << TYPE_SHIFT) | \
      (((user) & 7) << 6)          | \
      (((group) & 7) << 3)         | \
      ((other) & 7) )

#define RWX_ALL (I_R_OP | I_W_OP | I_X_OP)

typedef struct {
    int inodeNo;        /* the file i-node number */
    short type;         /* the file i-node type */
    char links;         /* number of links to the i-node */
    int size;           /* file size in bytes */
    int numBlocks;      /* number of blocks used by the file */
	int owner_perms;
	int group_perms;
	int other_perms;
} fileStat;

struct directory_t {
	int location;	//	Sector number
	int size;		//	Size in number of sectors
};

#endif