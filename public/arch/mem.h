#ifndef INC_MEM_MANAGER
#define INC_MEM_MANAGER


//#include "arch_inc/mem.h"
#include "inc/types.h"

paddr_t alloc_pages(uint32_t n);

// Map a page vaddr to paddr
void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags);

paddr_t get_paddr_page_ind(uint32_t ind);

void direct_map_all_pages(uint32_t *table1, paddr_t start, uint32_t flags);
paddr_t direct_map_n_pages(uint32_t *table1, paddr_t start, uint32_t count, uint32_t flags);

paddr_t offset_map_n_pages(uint32_t *table1,paddr_t start, paddr_t offset, uint32_t count, uint32_t flags);

void switch_page_table(uint32_t *table_next, uint8_t * next_stack);




#endif /* !*/
