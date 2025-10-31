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


// extern char __kernel_base[], __kernel_base_end[], __free_ram[], __free_ram_end[], __stack_top[];

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


/*
Purpose: CR3 serves as the Page Directory Base Register (PDBR).
Usage: It holds the physical address of the first page directory for the current task.

Physical Addressing: The upper 20 bits of CR3 are used for the physical address, 
while the lower bits may be used for process-context identifiers if the PCIDE bit in CR4 is set.
*/
extern void pdt_set(uint32_t pdt_addr); // TODO: para cr3

// Reserva espacio estático para estructuras gen_pt_t simples
static gen_pt_t pt_pool[32];
static int pt_index = 0;

gen_pt_t* get_gen_table(void) {
    // Virtual memory for the generic table
    gen_pt_t *pt = &pt_pool[pt_index++];
    memset(pt, 0, sizeof(*pt));

    // Allocate the actual physical page directory
    paddr_t pde_paddr = alloc_pages(1);
    if (!pde_paddr) PANIC("out of memory (page_dir)");
    memset((void*)pde_paddr, 0, PAGE_SIZE);

    pt->paddr = pde_paddr;
    pt->root  = (void*)pde_paddr; // identity map today

    return pt;
}
