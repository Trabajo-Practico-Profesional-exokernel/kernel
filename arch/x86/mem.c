#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"

extern char __free_ram[], __free_ram_end[], __kernel_base[];

paddr_t alloc_pages(uint32_t n) {
    return 0;
}






void switch_page_table(uint32_t *table_next, uint8_t* next_stack){

}


void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags) {
}

void direct_map_all_pages(uint32_t *table1, paddr_t start, uint32_t flags){

}


paddr_t get_paddr_page_ind(uint32_t ind){
    return 0;

}

paddr_t direct_map_n_pages(uint32_t *table1,paddr_t start, uint32_t count, uint32_t flags){
    return 0;

}

paddr_t offset_map_n_pages(uint32_t *table1,paddr_t start, paddr_t offset, uint32_t count, uint32_t flags){
    return 0;

}