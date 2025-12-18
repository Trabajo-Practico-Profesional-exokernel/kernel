#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "paging.h" // Aporta page_info y page_manager
#include "std/string.h"

/*
Primera seccion no se mapea por seguridad, deteccion de bugs y simplicidad del kernel.

0x00000000 ──────────────────────────────
            (no mapeado / reservado)

0x01000000 ──────────────────────────────  <- VADDR_USER_BASE
            Codigo de usuario
            Datos
            BSS
            Heap (TODO:?)
            Stack de usuario

            ...
            (espacio usuario)

0xC0000000 ──────────────────────────────  <- VADDR_KERNEL_BASE
            Kernel text
            Kernel data
            Kernel stacks
            Page tables
            Dispositivos (framebuffer, etc.)

0xFFFFFFFF ──────────────────────────────
*/


extern char __free_ram[], __free_ram_end[], __kernel_base[], __kernel_base_end[];


struct page_info* free_pages;
struct page_manager main_page_table;
void page_free(struct page_info *page);

static paddr_t page_info_to_pa(struct page_info *page){
    paddr_t pa = (paddr_t)__free_ram + (page->index * PAGE_SIZE);
    return pa;
}

paddr_t get_next_free_page(){
    if (main_page_table.free_pages == 0){
        return 0;
    }
    struct page_info *next_page_addr = main_page_table.free_page_list;
    
    main_page_table.free_page_list = next_page_addr->next_free_page;
    main_page_table.free_pages --;

    next_page_addr->ref +=1;
    next_page_addr->next_free_page = NULL;

    paddr_t pa = next_page_addr->pa;

    memset((void*)pa, 0, PAGE_SIZE);

    return pa;
}

void switch_page_table(uint32_t pde_paddr) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(pde_paddr) : "memory");
}

void mem_init(void) {
    paddr_t low_free_dir = (paddr_t)__free_ram;
    paddr_t high_free_dir = (paddr_t)__free_ram_end;
    printf("FREE RAM IS %x\n", low_free_dir);
    uint32_t total_free_pages = (high_free_dir - low_free_dir) / PAGE_SIZE;
    uint32_t map_size = total_free_pages * sizeof(struct page_info);

    // reservo el array de paginas al inicio de la memoria ram libre
    main_page_table.pages_array = (struct page_info*) low_free_dir;

    memset((void *) main_page_table.pages_array, 0, map_size);

    // las paginas fisicas utilizables comienzan despues de la tabla de paginas
    paddr_t pool_start = (paddr_t) ROUNDUP(low_free_dir + map_size, PAGE_SIZE);

    main_page_table.free_page_list = NULL;
    main_page_table.free_pages = 0;
    uint32_t index = 0;

    for (paddr_t pa = pool_start; pa < high_free_dir; pa += PAGE_SIZE) {
        
        // obtengo el puntero al a la pagina (que ya existe en el array)
        struct page_info* descriptor = &main_page_table.pages_array[index];
        if (pa + PAGE_SIZE < high_free_dir){
            descriptor->next_free_page = &main_page_table.pages_array[index+1];
        } else {
            descriptor->next_free_page = NULL;
        }

        descriptor->pa = pa;
        index ++;
        main_page_table.free_pages++;
        if (main_page_table.free_page_list == NULL){
            main_page_table.free_page_list = descriptor;
        }
    }
}

#define MAX_ROLLBACK_ALLOC 16

//corregir las paginas que fueron tomadas pero no referenciadas (hay un leek de memoria para n mayor a 1)
paddr_t alloc_pages(uint32_t n) {
    if (n == 0) return 0;
    
    // Lista para el rollback
    paddr_t allocated_list[MAX_ROLLBACK_ALLOC];
    if (n > MAX_ROLLBACK_ALLOC) {
        PANIC("alloc_pages: n excede MAX_ROLLBACK_ALLOC");
    }

    uint32_t num_allocated = 0;
    paddr_t pa = 0;

    while (num_allocated < n) {
        paddr_t next_paddr = get_next_free_page(); // Llama a [cite: 137-139]

        if (next_paddr == 0) {
            for (uint32_t i = 0; i < num_allocated; i++) {
                //struct page_info *page_to_free = page_info_to_pa(allocated_list[i]);
                page_free((struct page_info *) allocated_list[i]);
            }
            PANIC("alloc_pages: out of memory");
        }

        allocated_list[num_allocated] = next_paddr;
        if (num_allocated == 0) {
            pa = next_paddr;
        }
        num_allocated++;
    }

    return pa;
}

void page_free(struct page_info *page) {
    if (page == NULL) {
        PANIC("free page: page is NULL");
    }

    if (page->ref > 0) {
        page->ref--;
    }

    if (page->ref != 0) {
        return; // La página sigue referenciada, no liberar
    }

    if (page->next_free_page != NULL) {
        PANIC("free page: next free page no es NULL (double free?)");
    }

    page->next_free_page = main_page_table.free_page_list;
    main_page_table.free_page_list = page;
    main_page_table.free_pages++;
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

// TODO: added para que compile, chequear si funciona bien
paddr_t get_paddr_for(uint32_t *page_directory, vaddr_t vaddr) {
    uint32_t pde_index = PAGE_DIRECTORY_INDEX(vaddr);
    uint32_t pte_index = PAGE_TABLE_INDEX(vaddr);
    uint32_t offset    = vaddr & 0xFFF;

    pd_entry pde = page_directory[pde_index];
    if (!(pde & I86_PDE_PRESENT)) {
        return 0; // page fault
    }

    struct ptable *pt = (struct ptable *)
        PAGE_GET_PHYSICAL_ADDRESS(pde);

    pd_entry pte = pt->m_entries[pte_index];
    if (!(pte & I86_PTE_PRESENT)) {
        return 0; // page fault
    }

    paddr_t phys_base = PAGE_GET_PHYSICAL_ADDRESS(pte);
    return phys_base + offset;
}