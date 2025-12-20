#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "std/string.h"
#include "arch_inc/virtio.h"

extern char __free_ram[], __free_ram_end[], __kernel_base[], __kernel_base_end[], __user_ram_end[];

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
    
    printf("SETTING UP KERNEL PAGETABLE at %x\n", kernel_page_table);
    printf("MAP IN KERNEL from kernel base to ram end: %x to %x\n", (paddr_t) __kernel_base, (paddr_t) __free_ram_end);
    direct_map_range(page_table, 
            (paddr_t) __kernel_base,
            // (paddr_t) __kernel_base_end,
            (paddr_t) __user_ram_end, // User ram end is the last mem used
            KERNEL_PERMISSIONS_ALL
    );

    map_page(page_table, VIRTIO_BLK_PADDR, VIRTIO_BLK_PADDR, PAGE_R | PAGE_W); 

}
void switch_to_kernel_tables(void){
    // printf("SHOULD SWITCH TO KERNEL PAGES? IS THAT IT? %x\n", (uint32_t *) kernel_page_table);

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

void map_page(uint32_t *table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags) {
    if (!is_aligned(vaddr, PAGE_SIZE))
        PANIC("unaligned vaddr %x", vaddr);

    if (!is_aligned(paddr, PAGE_SIZE))
        PANIC("unaligned paddr %x", paddr);


    // YO ESTO LO CAMBIARIA A QUE USE LA PDE EN VEZ DE HARDCODEARLO
    uint32_t vpn1 = (vaddr >> 22) & 0x3ff;
    if ((table1[vpn1] & PAGE_V) == 0) {
        // Create the 1st level page table if it doesn't exist.
        uint32_t pt_paddr = alloc_pages(1);
        table1[vpn1] = ((pt_paddr / PAGE_SIZE) << 10) | PAGE_V;
    }

    // ACA HARIA LA PTE
    // Set the 2nd level page table entry to map the physical page.
    uint32_t vpn0 = (vaddr >> 12) & 0x3ff;
    uint32_t *table0 = (uint32_t *) ((table1[vpn1] >> 10) * PAGE_SIZE);
    table0[vpn0] = ((paddr / PAGE_SIZE) << 10) | flags | PAGE_V;
}

paddr_t get_paddr_for(uint32_t *table1, vaddr_t vaddr) {
    uint32_t vpn1 = (vaddr >> 22) & 0x3ff;
    uint32_t vpn0 = (vaddr >> 12) & 0x3ff;
    uint32_t offset = vaddr & 0xfff; // Last 3 digits i.e 0 to 4095 offset in page

    uint32_t pde = table1[vpn1];
    if ((pde & PAGE_V) == 0) { // Directory page not mapped
        return 0; // or PANIC / page fault
    }

    // Get second-level page table physical address
    paddr_t table0_paddr = ((pde >> 10) * PAGE_SIZE);

    // You must be able to access this page table in kernel VA space.
    // But we have direct mapping in kernel pages.. where you should call this from.
    uint32_t *table0 = (uint32_t *) table0_paddr;

    uint32_t pte = table0[vpn0];
    if ((pte & PAGE_V) == 0) { // Not mapped! second level page
        return 0; // page fault
    }
    
    paddr_t page_paddr = ((pte >> 10) * PAGE_SIZE);
    return page_paddr + offset;
}




paddr_t direct_map_range(uint32_t *table1, paddr_t range_start, paddr_t range_end, uint32_t flags){
    paddr_t paddr = range_start;
    while (paddr < range_end){
        // printf("MAPPING PAGE %x < %x\n", paddr, range_end);
        map_page(table1, paddr, paddr, flags); // Direct map        
        paddr += PAGE_SIZE;
    }
    
    return paddr;
}

paddr_t offset_map_range(uint32_t *table1, paddr_t range_start, paddr_t range_end, 
                        vaddr_t mapped_vstart, uint32_t flags){
    paddr_t paddr = range_start;
    paddr_t vaddr = mapped_vstart;

    while (paddr < range_end){
        map_page(table1, vaddr, paddr, flags); // Map offseted to there        
        paddr += PAGE_SIZE;
        vaddr += PAGE_SIZE;
    }
    
    return paddr;
}