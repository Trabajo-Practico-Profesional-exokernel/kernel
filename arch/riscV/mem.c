#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"

extern char __free_ram[], __free_ram_end[], __kernel_base[];

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






void switch_page_table(uint32_t *table_next, uint8_t* next_stack){
    __asm__ __volatile__(
        "sfence.vma\n" // sfence.vma clears TLB cache, just in case?
        "csrw satp, %[satp]\n" // Write the index of physical page | constant for SATP
        "sfence.vma\n"  // sfence.vma clears TLB cache , to ensure no remaining map is old
        "csrw sscratch, %[sscratch]\n" // save the stack pointer just in case? who knows why 
        :
        : [satp] "r" (SATP_SV32 | ((uint32_t) table_next / PAGE_SIZE)),
          [sscratch] "r" ((uint32_t) next_stack)
    );

}


void map_page(uint32_t *table1, uint32_t vaddr, paddr_t paddr, uint32_t flags) {
    if (!is_aligned(vaddr, PAGE_SIZE))
        PANIC("unaligned vaddr %x", vaddr);

    if (!is_aligned(paddr, PAGE_SIZE))
        PANIC("unaligned paddr %x", paddr);

    uint32_t vpn1 = (vaddr >> 22) & 0x3ff;
    if ((table1[vpn1] & PAGE_V) == 0) {
        // Create the 1st level page table if it doesn't exist.
        uint32_t pt_paddr = alloc_pages(1);
        table1[vpn1] = ((pt_paddr / PAGE_SIZE) << 10) | PAGE_V;
    }

    // Set the 2nd level page table entry to map the physical page.
    uint32_t vpn0 = (vaddr >> 12) & 0x3ff;
    uint32_t *table0 = (uint32_t *) ((table1[vpn1] >> 10) * PAGE_SIZE);
    table0[vpn0] = ((paddr / PAGE_SIZE) << 10) | flags | PAGE_V;
}

void direct_map_all_pages(uint32_t *table1, paddr_t start, uint32_t flags){
    for (paddr_t paddr = start;
        paddr < (paddr_t) __free_ram_end; paddr += PAGE_SIZE){

        //printf("MAPPED all page direct now at %x\n", paddr);
        map_page(table1, paddr, paddr, flags);
    }
}


paddr_t get_paddr_page_ind(uint32_t ind){
    return ((paddr_t) __kernel_base) + (PAGE_SIZE * ind);
}

paddr_t direct_map_n_pages(uint32_t *table1,paddr_t start, uint32_t count, uint32_t flags){
    paddr_t paddr = start;
    uint32_t ind = 0;
    while (paddr < (paddr_t) __free_ram_end && ind < count){
        map_page(table1, paddr, paddr, flags);        
        //printf("MAPPED %u page direct at %x\n", ind, paddr);
        paddr += PAGE_SIZE;
        ind+=1;
    }

    return paddr;
}

paddr_t offset_map_n_pages(uint32_t *table1,paddr_t start, paddr_t offset, uint32_t count, uint32_t flags){
    paddr_t paddr = start;
    uint32_t ind = 0;
    while (paddr < (paddr_t) __free_ram_end && ind < count){
        map_page(table1, offset+paddr, paddr, flags);        
        paddr += PAGE_SIZE;
        ind+=1;
    }

    return paddr;
}