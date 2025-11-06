#ifndef INC_MEM_MANAGER
#define INC_MEM_MANAGER


//#include "arch_inc/mem.h"
#include "inc/types.h"

typedef struct GenericPageTable {
    void *root;     // Virtual address of page directory root (pde for x86)
    paddr_t paddr;  // Physical address of that root (for satp or cr3)
} gen_pt_t;


void mem_init(void);
paddr_t get_next_free_page();
paddr_t alloc_pages(uint32_t n);

// Map a page vaddr to paddr
void map_page(gen_pt_t *table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags);

paddr_t get_paddr_page_ind(uint32_t ind);
paddr_t get_paddr_last_page();

paddr_t get_paddr_kernel_start();
paddr_t get_paddr_kernel_end();

void switch_page_table(uint32_t *table_next, uint8_t * next_stack);

paddr_t direct_map_range(gen_pt_t *table1, paddr_t range_start, paddr_t range_end, uint32_t flags);
paddr_t offset_map_range(gen_pt_t *table1, paddr_t range_start, paddr_t range_end, 
						vaddr_t mapped_vstart, uint32_t flags);

//void direct_map_all_pages(uint32_t *table1, paddr_t start, uint32_t flags);

#endif /* !*/
