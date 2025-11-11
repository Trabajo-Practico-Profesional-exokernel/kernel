#ifndef PAGING_H
#define PAGING_H

#include "inc/types.h"
#include "arch/mem.h"

#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR	1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

/*
Each process (including the kernel) has a page directory.
In the page directory, each entry points to a page table. 
In the page table, each entry points to a 4 KiB physical page frame.

Each page directory has 1024 entries (PDEs).
Each page table also has 1024 entries (PTEs).
1024 * 1024 * 4KiB = 4GB

Address translation involves dividing the virtual address into three parts: 
    - the most significant 10 bits (bits 22-31) specify the index of the page directory entry
    - the next 10 bits (bits 12-21) specify the index of the page table entry
    - the least significant 12 bits (bits 0-11) specify the page offset
*/
struct pdirectory {
	uint32_t m_entries[PAGES_PER_DIR];
};

struct ptable {
	uint32_t m_entries[PAGES_PER_TABLE];
};

enum PAGE_PTE_FLAGS {
	I86_PTE_PRESENT			=	1,			//0000000000000000000000000000001
	I86_PTE_WRITABLE		=	2,			//0000000000000000000000000000010
	I86_PTE_USER			=	4,			//0000000000000000000000000000100
	I86_PTE_WRITETHOUGH		=	8,			//0000000000000000000000000001000
	I86_PTE_NOT_CACHEABLE	=	0x10,		//0000000000000000000000000010000
	I86_PTE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PTE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PTE_PAT				=	0x80,		//0000000000000000000000010000000
	I86_PTE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PTE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PTE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};

enum PAGE_PDE_FLAGS {

	I86_PDE_PRESENT			=	1,			//0000000000000000000000000000001
	I86_PDE_WRITABLE		=	2,			//0000000000000000000000000000010
	I86_PDE_USER			=	4,			//0000000000000000000000000000100
	I86_PDE_PWT				=	8,			//0000000000000000000000000001000
	I86_PDE_PCD				=	0x10,		//0000000000000000000000000010000
	I86_PDE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PDE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PDE_4MB				=	0x80,		//0000000000000000000000010000000
	I86_PDE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PDE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PDE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};

/*
Page information saved here
*/
struct page_info {
    uint8_t index;
    uint8_t ref;
    paddr_t pa;
    struct page_info* next_free_page;
};

struct page_manager {
    struct page_info* free_page_list;
    struct page_info* pages_array;
    uint32_t free_pages;
};

extern struct page_manager main_page_table;


/*
Purpose: CR3 serves as the Page Directory Base Register (PDBR).
Usage: It holds the physical address of the first page directory for the current task.

Physical Addressing: The upper 20 bits of CR3 are used for the physical address, 
while the lower bits may be used for process-context identifiers if the PCIDE bit in CR4 is set.
*/


#endif /* PAGING_H */
