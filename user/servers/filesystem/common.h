/* common.h */

#ifndef COMMON_H
#define COMMON_H

// Integración: Incluir definiciones de tipos y E/S del kernel
#include "inc/types.h"
#include "std/printf.h"

#ifndef NULL
#define NULL ((void*) 0)
#endif

// Tamaño del sector
#define SECTOR_SIZE 512

/* ASSERT adaptado para espacio de usuario.
   Usa printf en lugar de acceso directo a memoria de video.
   Elimina 'cli' ya que es una instrucción privilegiada.
*/
#define ASSERT2(p, s) \
	do { \
	if (!(p)) { \
		printf("Assertion failure: %s\n", s); \
		printf("file: %s\n", __FILE__); \
		printf("line: %d\n", __LINE__); \
		while (1); /* Hang process */ \
	} \
	} while(0)

#define ASSERT(p) ASSERT2(p, #p)

// Halt ahora simplemente fuerza el fallo de aserción
#define HALT(s) ASSERT2(FALSE, s)

// Typedefs

typedef enum {
	FALSE, TRUE
} bool_t;

// Asumimos que inc/types.h define los tipos estándar (int8_t, uint32_t, etc.).
// Si el entorno de compilación no los provee, descomentar las líneas necesarias.
/*
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
*/

// Tipos específicos usados por simple-linux-fs
typedef unsigned char uchar_t;
typedef unsigned int uint_t;
typedef unsigned long ulong_t;

#define FREE_INODE 0
#define DIRECTORY 1
#define FILE_TYPE 2

#define FS_O_RDONLY 1
#define FS_O_WRONLY 2
#define FS_O_RDWR 3

typedef struct {
    int inodeNo;        /* the file i-node number */
    short type;         /* the file i-node type */
    char links;         /* number of links to the i-node */
    int size;           /* file size in bytes */
    int numBlocks;      /* number of blocks used by the file */
} fileStat;

struct directory_t {
	int location;	//	Sector number
	int size;		//	Size in number of sectors
};

#endif