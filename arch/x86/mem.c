#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"


// EN TEORIA ES LO MISMO QUE EN RISCV
extern char __free_ram[], __free_ram_end[], __kernel_base[], __kernel_base_end[];

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


paddr_t direct_map_range(uint32_t *table1, paddr_t range_start, paddr_t range_end, uint32_t flags){
    paddr_t paddr = range_start;
    while (paddr < range_end){
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


void switch_page_table(uint32_t *table_next, uint8_t* next_stack){

}

void map_page(uint32_t *page_dir, vaddr_t vaddr, paddr_t paddr, uint32_t perms) {
    
    // 1. Calcular el índice del Page Directory (PDI)
    // Usamos los 10 bits superiores de la vaddr (bits 31-22)
    uint32_t pdi = vaddr >> 22;

    // 2. Obtener la Page Directory Entry (PDE)
    uint32_t *pde_ptr = &page_dir[pdi];

    // 3. Verificar si la Page Table (el "cajón") existe
    // Si el bit "Present" (P) de la PDE es 0, necesitamos crear una Page Table nueva.
    if (!(*pde_ptr & PAGE_P_PRESENT)) {
        // No existe, pedir una página física vacía para usarla como Page Table
        paddr_t new_pt_addr = alloc_pages(1);
        if (new_pt_addr == 0) {
            PANIC("map_page: ¡No hay memoria para una nueva Page Table!");
        }
        
        // Limpiarla (importante, para que todas las PTEs empiecen como "no presentes")
        memset((void *)new_pt_addr, 0, PAGE_SIZE);

        // Actualizar la PDE para que apunte a nuestra nueva Page Table
        // La marcamos como Presente, Escribible y accesible por el Usuario.
        *pde_ptr = new_pt_addr | PAGE_P_PRESENT | PAGE_P_READ_WRITE | PAGE_P_USER;
    }

    // 4. Obtener la Page Table (PT)
    // La dirección de la PT está en los 20 bits superiores de la PDE
    uint32_t *page_table = (uint32_t *)(*pde_ptr & PAGE_ADDR_MASK);

    // 5. Calcular el índice de la Page Table (PTI)
    // Usamos los 10 bits del medio de la vaddr (bits 21-12)
    uint32_t pti = (vaddr >> 12) & 0x3FF; // 0x3FF es una máscara para 10 bits

    // 6. Obtener la Page Table Entry (PTE) y configurarla
    uint32_t *pte_ptr = &page_table[pti];
    
    // Mapeamos la dirección física (paddr) con los permisos dados
    // y la marcamos como Presente.
    *pte_ptr = (paddr & PAGE_ADDR_MASK) | perms | PAGE_P_PRESENT;
}