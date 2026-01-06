#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "std/string.h"
#include "arch_inc/virtio.h"
#include "arch_inc/virtio_blk.h"

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


void switch_page_table(uint32_t * pde_table) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(pde_table) : "memory");
}

void enable_paging(void) {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // bit PG (bit 31)
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
    printf("PG=1\n");
}




pd_entry* kernel_pde; // Physical address for Page Directory table

// NO HACE FALTA!
// pd_entry* very_initial_pde; // Physical address for Page Directory table for the initial paging when enabling


#define KERNEL_PERMISSIONS_ALL (I86_PTE_WRITABLE)
#define USER_PERMISSIONS_ALL (I86_PTE_WRITABLE | I86_PTE_USER)

void mem_init(void){
    // kernel_page_table = alloc_pages(1); // Para 0xC0000000 (Kernel)
    kernel_pde = (pd_entry*) alloc_pages(1); // Para 0x00000000 (Identity Map)

    if (kernel_pde == 0){
        PANIC("virtual mem init: out of memory for PTs");
    }

    direct_map_range(kernel_pde, // Map first 4MB for booting stuff
            (paddr_t) 0x0,
            (paddr_t) (1024 * PAGE_SIZE), 
            0
    );

    map_page(kernel_pde, VIRTIO_BLK_PADDR, VIRTIO_BLK_PADDR, KERNEL_PERMISSIONS_ALL); 
    // offset_map_range(kernel_pde, // Map first 4MB of kernel to 0xC000000 == VADDR_KERNEL_BASE
    //         (paddr_t) __kernel_base,
    //         (paddr_t) __kernel_base + (paddr_t) (1024 * PAGE_SIZE), 
    //         USER_PERMISSIONS_ALL,
    //         VADDR_KERNEL_BASE
    // );

    // No memory save attempt! map all available pages!
    direct_map_range(kernel_pde, 
            (paddr_t) __kernel_base,
            (paddr_t) __trampoline_end,
            KERNEL_PERMISSIONS_ALL
    );

    switch_page_table(kernel_pde);

    enable_paging();

    printf("---> ENABLED PAGING ALL OK\n");
}

uint32_t * init_user_pde_table(void){
    pd_entry * pd_table = alloc_pages(1);
    
    direct_map_range(pd_table, // Map first 4MB for booting stuff
            (paddr_t) 0x0,
            (paddr_t) (1024 * PAGE_SIZE), 
            KERNEL_PERMISSIONS_ALL
    );

    return pd_table;
}

void switch_to_kernel_tables(void){
    switch_page_table((uint32_t *) kernel_pde);
}


void map_page(uint32_t *pd_table, vaddr_t vaddr, paddr_t paddr, uint32_t permissions) {
    // if (!is_aligned(vaddr, PAGE_SIZE))
    //     PANIC("unaligned vaddr %x", vaddr);

    // if (!is_aligned(paddr, PAGE_SIZE))
    //     PANIC("unaligned paddr %x", paddr);


    uint32_t pd_index = GET_INDEX_IN_PAGE_DIRECTORY(vaddr);

    if (IS_NOT_PRESENT(pd_table[pd_index])) {

        // Alloc/Create the table for pages, i.e the page table at this index.
        // The page table is 1024 page table entries , of 32 bits each. i.e 4KB == 1 PAGE 
        paddr_t pt_paddr = alloc_pages(1);

        pd_table[pd_index] = (I86_PDE_FRAME & pt_paddr) | I86_PTE_PRESENT | I86_PDE_WRITABLE | I86_PDE_USER;
        
        // Other options
        // pd_table[pd_index] = I86_PTE_PRESENT | I86_PDE_WRITABLE;
        // pd_table[pd_index] = (pd_table[pd_index] & ~I86_PDE_FRAME) | pt_paddr;

        // pd_table[pd_index] = I86_PDE_FRAME & pt_paddr;
        // pd_table[pd_index] |= I86_PTE_PRESENT | I86_PDE_WRITABLE;

        printf("Allocated page at 0x%x for ptable pd_index %u (%x) == %x \n", pt_paddr, pd_index, pd_index* 4, pd_table[pd_index]);
    }

    // 
    uint32_t pt_index = GET_INDEX_IN_PAGE_TABLE(vaddr);
    
    uint32_t* pt_table = (uint32_t *) (pd_table[pd_index] & I86_PDE_FRAME);

    pt_table[pt_index] = (I86_PTE_FRAME & paddr) | I86_PTE_PRESENT | permissions;

    // Other options
    // pt_table[pt_index] = I86_PTE_FRAME & paddr;
    // pt_table[pt_index] |= I86_PTE_PRESENT | permissions;
    
    // pt_table[pt_index] = I86_PTE_PRESENT | permissions;
    // pt_table[pt_index] = (pt_table[pt_index] & ~I86_PTE_FRAME) | paddr;

    if(pt_index == 4){
        printf("Allocate pt entry table %x, ind: %u (%x) == %x\n", pt_table, pt_index, pt_index* 4, pt_table[pt_index]);
    }
    // pt_table[pt_index] |= SET_ENTRY_OFFSET_PTE(paddr);

}

// TODO: check this func
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
        // printf("MAPPING PAGE %x < %x\n", paddr, range_end);
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
