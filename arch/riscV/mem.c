#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "std/string.h"
#include "arch_inc/virtio.h"

extern char __free_ram[], __free_ram_end[], __kernel_base[], __kernel_base_end[], __trampoline_end[];

paddr_t alloc_pages(uint32_t n) {
    // next_paddr === last allocated mem end
    static paddr_t next_paddr = (paddr_t) __free_ram;
    paddr_t paddr = next_paddr;
    next_paddr += n * PAGE_SIZE;

    if (next_paddr > (paddr_t) __free_ram_end)
        PANIC("out of memory");

    memset((void *) paddr, 0, n * PAGE_SIZE);
    return paddr;
}


paddr_t get_paddr_page_ind(uint32_t ind){
    return ((paddr_t) __free_ram) + (PAGE_SIZE * ind);
}
paddr_t get_paddr_last_page(){
    return (paddr_t) __free_ram_end;
}

paddr_t get_paddr_kernel_start(){
    return (paddr_t) __kernel_base;
}

paddr_t get_paddr_kernel_end(){
    return (paddr_t) __kernel_base_end;
}

paddr_t kernel_page_table; // Physical address for Page Directory: SATP for RISCV
#define KERNEL_PERMISSIONS_ALL (PAGE_R | PAGE_W | PAGE_X)

void mem_init(void){
    kernel_page_table = alloc_pages(1);

    uint32_t *page_table = (uint32_t *) kernel_page_table;

    // First map page for page table as direct map
    // map_page(page_table,kernel_page_table, kernel_page_table, KERNEL_PERMISSIONS_ALL);
    
    debug_printf("SETTING UP KERNEL PAGETABLE at %x\n", kernel_page_table);
    debug_printf("MAP IN KERNEL from kernel base to ram end: %x to %x\n", (paddr_t) __kernel_base, (paddr_t) __free_ram_end);
    direct_map_range(page_table, 
            (paddr_t) __kernel_base,
            (paddr_t) __trampoline_end, // User trampoline is the last thing in physical mem
            KERNEL_PERMISSIONS_ALL
    );

    map_page(page_table, VIRTIO_BLK_PADDR, VIRTIO_BLK_PADDR, PAGE_R | PAGE_W); 

}

uint32_t * init_user_pde_table(void) {
    return (uint32_t *)alloc_pages(1);
}

void switch_to_kernel_tables(void){
    // debug_printf("SHOULD SWITCH TO KERNEL PAGES? IS THAT IT? %x\n", (uint32_t *) kernel_page_table);

    switch_page_table((uint32_t *) kernel_page_table);
}


void switch_page_table(uint32_t *table_next){
    __asm__ __volatile__(
        "sfence.vma\n" // sfence.vma clears TLB cache, just in case?
        "csrw satp, %[satp]\n" // Write the index of physical page | constant for SATP
        "sfence.vma\n"  // sfence.vma clears TLB cache , to ensure no remaining map is old
        :
        : [satp] "r" (SATP_SV32 | ((uint32_t) table_next / PAGE_SIZE))
    );

}


/*
void switch_page_table(uint32_t *table_next, uint8_t* next_stack){
    __asm__ __volatile__(
        "csrw sscratch, %[sscratch]\n" // save the kernel stack pointer just
        "sfence.vma\n" // sfence.vma clears TLB cache, just in case?
        "csrw satp, %[satp]\n" // Write the index of physical page | constant for SATP
        "sfence.vma\n"  // sfence.vma clears TLB cache , to ensure no remaining map is old
        :
        : [satp] "r" (SATP_SV32 | ((uint32_t) table_next / PAGE_SIZE)),
          [sscratch] "r" ((uint32_t) next_stack)
    );

}
*/

void map_page(uint32_t *pd_table, vaddr_t vaddr, paddr_t paddr, uint32_t permissions) {
    if (!is_aligned(vaddr, PAGE_SIZE))
        PANIC("unaligned vaddr %x", vaddr);

    if (!is_aligned(paddr, PAGE_SIZE))
        PANIC("unaligned paddr %x", paddr);


    uint32_t pd_index = GET_INDEX_IN_PAGE_DIRECTORY(vaddr);

    if (IS_NOT_PRESENT(pd_table[pd_index])) {

        // Alloc/Create the table for pages, i.e the page table at this index.
        // The page table is 1024 page table entries , of 32 bits each. i.e 4KB == 1 PAGE 
        paddr_t pt_paddr = alloc_pages(1);
        
        pd_table[pd_index] = SET_ENTRY_OFFSET(pt_paddr);
    }

    // 
    uint32_t pt_index = GET_INDEX_IN_PAGE_TABLE(vaddr);
    
    uint32_t* pt_table = (uint32_t *) GET_ENTRY_OFFSET(pd_table[pd_index]); 
    
    pt_table[pt_index] = SET_ENTRY_OFFSET(paddr) | permissions;

}



paddr_t get_paddr_page(uint32_t *page_table, size_t ind_pte){

    uint32_t pte_config = page_table[ind_pte];
    if (IS_NOT_PRESENT(pte_config)) { // Not mapped! second level page
        return 0; // page fault
    }
    return (paddr_t) GET_ENTRY_OFFSET(pte_config);
}

paddr_t get_paddr_page_table(uint32_t *page_directory, size_t ind_pde){
    uint32_t page_table_config = page_directory[ind_pde];
    
    // Directory page not mapped
    if (IS_NOT_PRESENT(page_table_config)) { 
        return 0; // or PANIC / page fault
    }
    
    return (paddr_t) GET_ENTRY_OFFSET(page_table_config); 
}

void get_vaddr_indexs(vaddr_t vaddr, size_t* ind_pde, size_t* ind_pte){
    *ind_pde = GET_INDEX_IN_PAGE_DIRECTORY(vaddr); // bits 0 to 9
    *ind_pte = GET_INDEX_IN_PAGE_TABLE(vaddr); // bits 10 to 19
}


paddr_t get_paddr_for(uint32_t *pd_table, vaddr_t vaddr) {
    uint32_t pd_index = GET_INDEX_IN_PAGE_DIRECTORY(vaddr); // bits 0 to 9
    uint32_t pt_index = GET_INDEX_IN_PAGE_TABLE(vaddr); // bits 10 to 19
    uint32_t pt_offset = GET_VADDR_OFFSET(vaddr);  // bits 20 to 31
    
    uint32_t page_table_config = pd_table[pd_index];
    
    // Directory page not mapped
    if (IS_NOT_PRESENT(page_table_config)) { 
        return 0; // or PANIC / page fault
    }
    
    uint32_t* pt_table = (uint32_t*) GET_ENTRY_OFFSET(page_table_config); 

    uint32_t pte_config = pt_table[pt_index];
    if (IS_NOT_PRESENT(pte_config)) { // Not mapped! second level page
        return 0; // page fault
    }
    
    paddr_t page_paddr = (paddr_t) GET_ENTRY_OFFSET(pte_config);
    return page_paddr + pt_offset;
}




paddr_t direct_map_range(uint32_t *pde_table, paddr_t range_start, paddr_t range_end, uint32_t permissions){
    paddr_t paddr = range_start;
    while (paddr < range_end){
        // debug_printf("MAPPING PAGE %x < %x\n", paddr, range_end);
        map_page(pde_table, paddr, paddr, permissions); // Direct map        
        paddr += PAGE_SIZE;
    }
    
    return paddr;
}

paddr_t offset_map_range(uint32_t *pde_table, paddr_t range_start, paddr_t range_end, uint32_t permissions, vaddr_t mapped_vstart){
    paddr_t paddr = range_start;
    paddr_t vaddr = mapped_vstart;

    while (paddr < range_end){
        map_page(pde_table, vaddr, paddr, permissions); // Map offseted to there        
        paddr += PAGE_SIZE;
        vaddr += PAGE_SIZE;
    }
    
    return paddr;
}