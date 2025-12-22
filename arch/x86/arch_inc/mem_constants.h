#ifndef INC_MEM_CONSTANTS
#define INC_MEM_CONSTANTS

#include "inc/types.h"

#define VADDR_KERNEL_BASE 0xC0000000
/*
* Constants for mapping pages/ virtual memory!
*/

// Máscara para obtener la dirección física de 20 bits (alineada a 4K)
#define PAGE_ADDR_MASK 0xFFFFF000

// TODO: make the virtual memory layout

#define KERNBASE        0xF0000000
#define KSTACKTOP       KERNBASE
#define KSTKSIZE        (8*PAGE_SIZE)
#define KSTKGAP         (8*PAGE_SIZE)
#define KSTACKTOPCPU(i) (KSTACKTOP - (i) * (KSTKSIZE + KSTKGAP))


// --- BITS DE PERMISOS (se pueden combinar) ---
#define PAGE_P_PRESENT    0x001 // Bit 0: Página está presente en memoria
#define PAGE_P_READ_WRITE 0x002 // Bit 1: 0=Solo Lectura, 1=Lectura/Escritura
#define PAGE_P_USER       0x004 // Bit 2: 0=Supervisor (Kernel), 1=Usuario (Proceso)
#define PAGE_P_WRITE_THRU 0x008 // Bit 3: Page-level write-through
#define PAGE_P_CACHE_DIS  0x010 // Bit 4: Page-level cache disable
#define PAGE_P_ACCESSED   0x020 // Bit 5: (Usado por la CPU) Fue accedida
#define PAGE_P_DIRTY      0x040 // Bit 6: (Usado por la CPU) Fue escrita (solo en PTE)
#define PAGE_P_PAGE_SIZE  0x080 // Bit 7: (Solo en PDE) 0=Página de 4KiB, 1=Página de 4MiB


///// From guide
/*
On the x86 architecture, the virtual address format actually uses three sections instead of two: The entry number in a page directory table,
the page table index, and the offset into that page. 
AAAAAAAAAA         BBBBBBBBBB        CCCCCCCCCCCC
directory index    page table index  offset into page

Page Table Entries (PTE) 

A page table entry is what represents a page, manages 4KB of physical mem format in x86 is 32 bits=

    Bit 0 (P): Present flag
        0: Page is not in memory
        1: Page is present (in memory)
    Bit 1 (R/W): Read/Write flag
        0: Page is read only
        1: Page is writable
    Bit 2 (U/S):User mode/Supervisor mode flag
        0: Page is kernel (supervisor) mode
        1: Page is user mode. Cannot read or write supervisor pages
    Bits 3-4 (RSVD): Reserved by Intel
    Bit 5 (A): Access flag. Set by processor
        0: Page has not been accessed
        1: Page has been accessed
    Bit 6 (D): Dirty flag. Set by processor
        0: Page has not been written to
        1: Page has been written to
    Bits 7-8 (RSVD): Reserved
    Bits 9-11 (AVAIL): Available for use
    Bits 12-31 (FRAME): Frame address == physical addr
*/

enum PAGE_PTE_FLAGS {
	I86_PTE_PRESENT			=	1,  		//0000000000000000000000000000001
	I86_PTE_WRITABLE		=	2,	    	//0000000000000000000000000000010
	I86_PTE_USER			=	4,		    //0000000000000000000000000000100
	I86_PTE_WRITETHOUGH		=	8,		    //0000000000000000000000000001000
	I86_PTE_NOT_CACHEABLE   =	0x10,		//0000000000000000000000000010000
	I86_PTE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PTE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PTE_PAT 			=	0x80,		//0000000000000000000000010000000
	I86_PTE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PTE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PTE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};



/* Page directory entry == arr of 1024 page entries has config of 32bits =
    Bit 0 (P): Present flag
        0: Page is not in memory
        1: Page is present (in memory)
    Bit 1 (R/W): Read/Write flag
        0: Page is read only
        1: Page is writable
    Bit 2 (U/S):User mode/Supervisor mode flag
        0: Page is kernel (supervisor) mode
        1: Page is user mode. Cannot read or write supervisor pages
    Bit 3 (PWT):Write-through flag
        0: Write back caching is enabled
        1: Write through caching is enabled
    Bit 4 (PCD):Cache disabled
        0: Page table will not be cached
        1: Page table will be cached
    Bit 5 (A): Access flag. Set by processor
        0: Page has not been accessed
        1: Page has been accessed
    Bit 6 (D): Reserved by Intel
    Bit 7 (PS): Page Size
        0: 4 KB pages
        1: 4 MB pages
    Bit 8 (G): Global Page (Ignored)
    Bits 9-11 (AVAIL): Available for use
    Bits 12-31 (FRAME): Page Table Base address
*/
// PDE page directory entry == arr of 1024
enum PAGE_PDE_FLAGS {
 
	I86_PDE_PRESENT			=	1,		    //0000000000000000000000000000001
	I86_PDE_WRITABLE		=	2,		    //0000000000000000000000000000010
	I86_PDE_USER			=	4,	    	//0000000000000000000000000000100
	I86_PDE_PWT	    		=	8,          //0000000000000000000000000001000
	I86_PDE_PCD		    	=	0x10,		//0000000000000000000000000010000
	I86_PDE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PDE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PDE_4MB		    	=	0x80,		//0000000000000000000000010000000
	I86_PDE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PDE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PDE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};

/// Now....
//! i86 architecture defines 1024 entries per table--do not change
#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024

//! page sizes are 4k
#define PAGE_SIZE 4096


#define GET_INDEX_IN_PAGE_DIRECTORY(vaddr) ((vaddr >> 22) & 0x3ff)
#define GET_INDEX_IN_PAGE_TABLE(vaddr) ((vaddr >> 12) & 0x3ff)
#define GET_VADDR_OFFSET(vaddr) (vaddr & 0xfff)

// #define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff) // get entry frame?


#define IS_NOT_PRESENT(entry) ((entry & PAGE_P_PRESENT) == 0)
#define IS_PRESENT(entry) ((entry & PAGE_P_PRESENT) != 0)

// This is used for getting both, the page table offset in page directory and page physical offset in page table 
// In an entry each entry manages 4KB or so of memory ... in case of page directories it would compound
#define GET_ENTRY_OFFSET(vaddr) ((vaddr >> 10) * PAGE_SIZE)

// This method gets the offset of an physical address, and puts it on the config. Also marks IS_PRESENT
// This is used in both page table entries and page directory entries! 
#define SET_ENTRY_OFFSET(paddr) (((paddr / PAGE_SIZE) << 10) | I86_PDE_PRESENT)

#define PTABLE_ADDR_SPACE_SIZE 0x400000

#define DTABLE_ADDR_SPACE_SIZE 0x100000000

typedef uint32_t pt_entry;
typedef uint32_t pd_entry;


// ptables and pdirectories are just arrays of entries. they are 4kb of size! == 1 PAGE
typedef pt_entry* ptable_t;
typedef pd_entry* pdirectory_t;

#endif /* !*/
