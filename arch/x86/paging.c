#include "paging.h"
#include "inc/types.h"
#include "inc/common.h"
#include "arch_inc/mem_constants.h"

#define NUM_ENTRIES 1024
#define PDT_SIZE NUM_ENTRIES * sizeof(pde_t)
#define PT_SIZE  NUM_ENTRIES * sizeof(pte_t)

#define VIRTUAL_TO_PDT_IDX(a) ((a >> 20) & 0x3FF)

#define PS_4KB 0x00
#define PS_4MB 0x01

#define IS_ENTRY_PRESENT(e) ((e)->config && 0x01)


extern char __kernel_base[], __kernel_base_end[], __free_ram[], __free_ram_end[], __stack_top[];

/*
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


