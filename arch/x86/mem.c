#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "paging.h"

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


paddr_t direct_map_range(gen_pt_t *table1, paddr_t range_start, paddr_t range_end, uint32_t flags){
    paddr_t paddr = range_start;
    while (paddr < range_end){
        map_page(table1, paddr, paddr, flags); // Direct map        
        paddr += PAGE_SIZE;
    }
    
    return paddr;
}

paddr_t offset_map_range(gen_pt_t *table1, paddr_t range_start, paddr_t range_end, 
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


/**
 * Función para "empaquetar" una dirección y permisos en una struct pte_t.
 * Es una función de ayuda para no repetir código.
 */
static void set_pte_entry(pte_t *pte, paddr_t paddr, uint32_t perms) {
    // 1. Construir el valor completo de 32 bits
    uint32_t entry = (paddr & PAGE_ADDR_MASK) | perms | PAGE_P_PRESENT;

    // 2. "Empaquetar" ese valor en los campos de la struct
    pte->config = (entry & 0xFF);           // Byte 0 (bits 0-7)
    pte->middle = (entry >> 8) & 0xFF;    // Byte 1 (bits 8-15)
    pte->high_addr = (entry >> 16) & 0xFFFF;  // Bytes 2 y 3 (bits 16-31)
}

/**
 * Función para "empaquetar" una dirección y permisos en una struct pde_t.
 */
static void set_pde_entry(pde_t *pde, paddr_t pt_addr, uint32_t perms) {
    // 1. Construir el valor completo de 32 bits
    uint32_t entry = (pt_addr & PAGE_ADDR_MASK) | perms;

    // 2. "Empaquetar" ese valor en los campos de la struct
    pde->config = (entry & 0xFF);           // Byte 0
    pde->low_addr = (entry >> 8) & 0xFF;    // Byte 1
    pde->high_addr = (entry >> 16) & 0xFFFF;  // Bytes 2 y 3
}

/**
 * Función para "desempaquetar" la dirección física de una pde_t.
 */
static paddr_t get_pde_addr(pde_t *pde) {
    // Reconstruir el valor de 32 bits desde los campos de la struct
    uint32_t entry = pde->config | (pde->low_addr << 8) | (pde->high_addr << 16);
    
    // Devolver solo la parte de la dirección
    return (entry & PAGE_ADDR_MASK);
}

void map_page(gen_pt_t *gen_pt, vaddr_t vaddr, paddr_t paddr, uint32_t perms) {
    pde_t *page_dir = (pde_t *) gen_pt->root;
   
    // 1. Índice del Page Directory
    uint32_t pdi = (vaddr >> 22) & 0x3FF;

    // 2. Entrada del Page Directory
    pde_t *pde_ptr = &page_dir[pdi];

    // 3. Crear Page Table si no existe
    if (!(pde_ptr->config & PAGE_P_PRESENT)) {
        paddr_t new_pt_addr = alloc_pages(1);
        memset((void *)new_pt_addr, 0, PAGE_SIZE);

        uint32_t pde_perms = PAGE_P_PRESENT | PAGE_P_READ_WRITE | PAGE_P_USER;
        set_pde_entry(pde_ptr, new_pt_addr, pde_perms);
    }

    // 4. Obtener la dirección física de la PT
    paddr_t pt_addr = get_pde_addr(pde_ptr);
    pte_t *page_table = (pte_t *)pt_addr;

    // 5. Índice del Page Table
    uint32_t pti = (vaddr >> 12) & 0x3FF;

    // 6. Configurar la entrada PTE
    pte_t *pte_ptr = &page_table[pti];
    set_pte_entry(pte_ptr, paddr, perms);
}