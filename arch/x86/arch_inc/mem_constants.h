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


// --- BITS DE PERMISOS (se pueden combinar) ---
#define PAGE_P_PRESENT    0x001 // Bit 0: Página está presente en memoria
#define PAGE_P_READ_WRITE 0x002 // Bit 1: 0=Solo Lectura, 1=Lectura/Escritura
#define PAGE_P_USER       0x004 // Bit 2: 0=Supervisor (Kernel), 1=Usuario (Proceso)
#define PAGE_P_WRITE_THRU 0x008 // Bit 3: Page-level write-through
#define PAGE_P_CACHE_DIS  0x010 // Bit 4: Page-level cache disable
#define PAGE_P_ACCESSED   0x020 // Bit 5: (Usado por la CPU) Fue accedida
#define PAGE_P_DIRTY      0x040 // Bit 6: (Usado por la CPU) Fue escrita (solo en PTE)
#define PAGE_P_PAGE_SIZE  0x080 // Bit 7: (Solo en PDE) 0=Página de 4KiB, 1=Página de 4MiB

// Máscara para obtener la dirección física de 20 bits (alineada a 4K)
#define PAGE_ADDR_MASK 0xFFFFF000

#endif /* !*/
