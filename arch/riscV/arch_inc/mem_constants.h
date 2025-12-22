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



/*
In riscv It also a structure in 2 levels and we have the mapping of 32 bits vaddr to 3 parts
1 pde index, with another name in riscv = 10 bits == 0 to 9
1 pte index, with another name = 10 bits == 10 to 19
offset = 12 bits = 19 to 31
*/
#define GET_INDEX_IN_PAGE_DIRECTORY(vaddr) ((vaddr >> 22) & 0x3ff)
#define GET_INDEX_IN_PAGE_TABLE(vaddr) ((vaddr >> 12) & 0x3ff)
#define GET_VADDR_OFFSET(vaddr) (vaddr & 0xfff)


// PAGE_V is the analog for the bit IS_PRESENT in x86.
#define IS_NOT_PRESENT(entry) ((entry & PAGE_V) == 0)
#define IS_PRESENT(entry) ((entry & PAGE_V) != 0)


// This is used for getting both, the page table offset in page directory and page physical offset in page table 
// In an entry each entry manages 4KB or so of memory ... in case of page directories it would compound
#define GET_ENTRY_OFFSET(vaddr) ((vaddr >> 10) * PAGE_SIZE)

// This method gets the offset of an physical address, and puts it on the config. Also marks IS_PRESENT
// This is used in both page table entries and page directory entries! 
#define SET_ENTRY_OFFSET(paddr) (((paddr / PAGE_SIZE) << 10) | PAGE_V)


#endif /* !*/
