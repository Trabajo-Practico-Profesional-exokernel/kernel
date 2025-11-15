#ifndef INC_MEM_MANAGER
#define INC_MEM_MANAGER


//#include "arch_inc/mem.h"
#include "inc/types.h"

void pde_init();
void mem_init(void);
paddr_t get_next_free_page();
paddr_t alloc_pages(uint32_t n);

// Map a page vaddr to paddr
//void map_page(gen_pt_t *table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags); //TODO: CHANGE

paddr_t get_paddr_page_ind(uint32_t ind);
paddr_t get_paddr_last_page();

paddr_t get_paddr_kernel_start();
paddr_t get_paddr_kernel_end();

#ifdef IS_RISC
void switch_page_table(uint32_t *table_next, uint8_t * next_stack);
paddr_t direct_map_range(paddr_t *table1, paddr_t range_start, paddr_t range_end, uint32_t flags);
paddr_t offset_map_range(paddr_t *table1, paddr_t range_start, paddr_t range_end, 
						vaddr_t mapped_vstart, uint32_t flags);

void direct_map_all_pages(uint32_t *table1, paddr_t start, uint32_t flags);
void map_page(uint32_t *table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags);
#else
void switch_page_table(uint32_t pde_paddr);
#endif


#endif /* !*/
