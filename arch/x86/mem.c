// #include "inc/common.h"
// #include "arch_inc/mem_constants.h"
// #include "arch/mem.h"


// // EN TEORIA ES LO MISMO QUE EN RISCV
// extern char __free_ram[], __free_ram_end[], __kernel_base[], __kernel_base_end[];

// paddr_t alloc_pages(uint32_t n) {
//     // next_paddr === last allocated mem end
//     static paddr_t next_paddr = (paddr_t) __free_ram;
//     paddr_t paddr = next_paddr;
//     next_paddr += n * PAGE_SIZE;

//     if (next_paddr > (paddr_t) __free_ram_end)
//         PANIC("out of memory");

//     memset((void *) paddr, 0, n * PAGE_SIZE);
//     return paddr;
// }


// paddr_t get_paddr_page_ind(uint32_t ind){
//     return ((paddr_t) __free_ram) + (PAGE_SIZE * ind);
// }
// paddr_t get_paddr_last_page(){
//     return (paddr_t) __free_ram_end;
// }

// paddr_t get_paddr_kernel_start(){
//     return (paddr_t) __kernel_base;
// }

// paddr_t get_paddr_kernel_end(){
//     return (paddr_t) __kernel_base_end;
// }


// paddr_t direct_map_range(uint32_t *table1, paddr_t range_start, paddr_t range_end, uint32_t flags){
//     paddr_t paddr = range_start;
//     while (paddr < range_end){
//         map_page(table1, paddr, paddr, flags); // Direct map        
//         paddr += PAGE_SIZE;
//     }
    
//     return paddr;
// }

// paddr_t offset_map_range(uint32_t *table1, paddr_t range_start, paddr_t range_end, 
//                         vaddr_t mapped_vstart, uint32_t flags){
//     paddr_t paddr = range_start;
//     paddr_t vaddr = mapped_vstart;

//     while (paddr < range_end){
//         map_page(table1, vaddr, paddr, flags); // Map offseted to there        
//         paddr += PAGE_SIZE;
//         vaddr += PAGE_SIZE;
//     }
    
//     return paddr;
// }





// void switch_page_table(uint32_t *table_next, uint8_t* next_stack){

// }


// void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags) {
// }

