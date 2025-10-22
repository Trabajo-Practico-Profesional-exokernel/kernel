#ifndef INC_MEM_CONSTANTS
#define INC_MEM_CONSTANTS

#include "inc/types.h"
#define PAGE_SIZE 4096
/*
* Constants for mapping pages/ virtual memory!
*/
#define SATP_SV32 (1u << 31)
#define PAGE_V    (1 << 0)   // "Valid" bit (entry is enabled)
#define PAGE_R    (1 << 1)   // Readable
#define PAGE_W    (1 << 2)   // Writable
#define PAGE_X    (1 << 3)   // Executable
#define PAGE_U    (1 << 4)   // User (accessible in user mode)



#endif /* !*/
