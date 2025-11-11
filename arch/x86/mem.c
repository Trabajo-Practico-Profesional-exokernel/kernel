// #include "inc/common.h"
// #include "arch_inc/mem_constants.h"
// #include "arch/mem.h"
// #include "paging.h"

// // EN TEORIA ES LO MISMO QUE EN RISCV
// extern char __free_ram[], __free_ram_end[], __kernel_base[], __kernel_base_end[];

// struct page_info* free_pages;
// struct page_manager main_page_table;

// static pde_t *kernel_pd_addr;

// static paddr_t page_info_to_pa(struct page_info *page){
//     paddr_t pa = (paddr_t)__free_ram + (page->index * PAGE_SIZE);
//     return pa;
// }

// paddr_t get_next_free_page(){
//     if (main_page_table.free_pages == 0){
//         return 0;
//     }
//     struct page_info *next_page_addr = main_page_table.free_page_list;
    
//     main_page_table.free_page_list = next_page_addr->next_free_page;
//     main_page_table.free_pages --;

//     next_page_addr->ref +=1;
//     next_page_addr->next_free_page = NULL;

//     paddr_t pa = next_page_addr->pa;

//     memset((void*)pa, 0, PAGE_SIZE);

//     return pa;
// }

// void load_cr3(uint32_t pde_paddr) {
//     __asm__ volatile("mov %0, %%cr3" :: "r"(pde_paddr) : "memory");
// }

// void pde_init(){
//     paddr_t p_pde = alloc_pages(1);
//     load_cr3(p_pde);
//     printf("---------ALOC PDE-----------\n");
//     kernel_pd_addr = (pde_t*) p_pde;

//     printf("Direccion Fisica: [%x]\n", p_pde );
//     printf("Puntero (Virtual): [%x]\n", kernel_pd_addr );
    
//     #define RECURSIVE_PDE_INDEX 1023
//     uint32_t perms = PAGE_P_PRESENT | PAGE_P_USER | PAGE_P_READ_WRITE;

//     pde_t *entry_ptr = &kernel_pd_addr[RECURSIVE_PDE_INDEX];

//     set_pde_entry(entry_ptr, p_pde, perms);

//     printf("Valor PDE[4095] (debug): config=[%x] high_addr=[%x] low_addr=[%x]\n", 
//         kernel_pd_addr[RECURSIVE_PDE_INDEX].config, 
//         kernel_pd_addr[RECURSIVE_PDE_INDEX].high_addr, 
//         kernel_pd_addr[RECURSIVE_PDE_INDEX].low_addr);

//     /* * NOTA: Esta función 'pde_init' aún está incompleta.
//      * Ahora debe usar 'direct_map_range' [cite: 167-170] para mapear 
//      * el código del kernel, la RAM física (__free_ram) y el 
//      * framebuffer (0xB8000) antes de activar CR3.
//      */
// }


// void mem_init(void) {
//     paddr_t low_free_dir = (paddr_t)__free_ram;
//     paddr_t high_free_dir = (paddr_t)__free_ram_end;
//     printf("FREE RAM IS %x\n", low_free_dir);
//     uint32_t total_free_pages = (high_free_dir - low_free_dir) / PAGE_SIZE;
//     uint32_t map_size = total_free_pages * sizeof(struct page_info);

//     // reservo el array de paginas al inicio de la memoria ram libre
//     main_page_table.pages_array = (struct page_info*) low_free_dir;

//     memset((void *) main_page_table.pages_array, 0, map_size);

//     // las paginas fisicas utilizables comienzan despues de la tabla de paginas
//     paddr_t pool_start = (paddr_t) ROUNDUP(low_free_dir + map_size, PAGE_SIZE);

//     main_page_table.free_page_list = NULL;
//     main_page_table.free_pages = 0;
//     uint32_t index = 0;

//     for (paddr_t pa = pool_start; pa < high_free_dir; pa += PAGE_SIZE) {
        
//         // obtengo el puntero al a la pagina (que ya existe en el array)
//         struct page_info* descriptor = &main_page_table.pages_array[index];
//         if (pa + PAGE_SIZE < high_free_dir){
//             descriptor->next_free_page = &main_page_table.pages_array[index+1];
//         } else {
//             descriptor->next_free_page = NULL;
//         }

//         descriptor->pa = pa;
//         index ++;
//         main_page_table.free_pages++;
//         if (main_page_table.free_page_list == NULL){
//             main_page_table.free_page_list = descriptor;
//         }
//     }
// }

// #define MAX_ROLLBACK_ALLOC 16

// //corregir las paginas que fueron tomadas pero no referenciadas (hay un leek de memoria para n mayor a 1)
// paddr_t alloc_pages(uint32_t n) {
//     if (n == 0) return 0;
    
//     // Lista para el rollback
//     paddr_t allocated_list[MAX_ROLLBACK_ALLOC];
//     if (n > MAX_ROLLBACK_ALLOC) {
//         PANIC("alloc_pages: n excede MAX_ROLLBACK_ALLOC");
//     }

//     uint32_t num_allocated = 0;
//     paddr_t pa = 0;

//     while (num_allocated < n) {
//         paddr_t next_paddr = get_next_free_page(); // Llama a [cite: 137-139]

//         if (next_paddr == 0) {
//             for (uint32_t i = 0; i < num_allocated; i++) {
//                 struct page_info *page_to_free = page_info_to_pa(allocated_list[i]);
//                 page_free(page_to_free);
//             }
//             PANIC("out of memory");
//         }

//         allocated_list[num_allocated] = next_paddr;
//         if (num_allocated == 0) {
//             pa = next_paddr;
//         }
//         num_allocated++;
//     }

//     return pa;
// }

// void page_free(struct page_info *page) {
//     if (page == NULL) {
//         PANIC("free page: page is NULL");
//     }

//     if (page->ref > 0) {
//         page->ref--;
//     }

//     if (page->ref != 0) {
//         return; // La página sigue referenciada, no liberar
//     }

//     if (page->next_free_page != NULL) {
//         PANIC("free page: next free page no es NULL (double free?)");
//     }

//     page->next_free_page = main_page_table.free_page_list;
//     main_page_table.free_page_list = page;
//     main_page_table.free_pages++;
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


// paddr_t direct_map_range(gen_pt_t *table1, paddr_t range_start, paddr_t range_end, uint32_t flags){
//     paddr_t paddr = range_start;
//     while (paddr < range_end){
//         map_page(table1, paddr, paddr, flags); // Direct map        
//         paddr += PAGE_SIZE;
//     }
    
//     return paddr;
// }

// paddr_t offset_map_range(gen_pt_t *table1, paddr_t range_start, paddr_t range_end, 
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


// /**
//  * Función para "empaquetar" una dirección y permisos en una struct pte_t.
//  * Es una función de ayuda para no repetir código.
//  */
// static void set_pte_entry(pte_t *pte, paddr_t paddr, uint32_t perms) {
//     // 1. Construir el valor completo de 32 bits
//     uint32_t entry = (paddr & PAGE_ADDR_MASK) | perms | PAGE_P_PRESENT;

//     // 2. "Empaquetar" ese valor en los campos de la struct
//     pte->config = (entry & 0xFF);           // Byte 0 (bits 0-7)
//     pte->middle = (entry >> 8) & 0xFF;    // Byte 1 (bits 8-15)
//     pte->high_addr = (entry >> 16) & 0xFFFF;  // Bytes 2 y 3 (bits 16-31)
// }

// /**
//  * Función para "empaquetar" una dirección y permisos en una struct pde_t.
//  */
// void set_pde_entry(pde_t *pde, paddr_t pt_addr, uint32_t perms) {
//     // 1. Construir el valor completo de 32 bits
//     uint32_t entry = (pt_addr & PAGE_ADDR_MASK) | perms;

//     // 2. "Empaquetar" ese valor en los campos de la struct
//     pde->config = (entry & 0xFF);           // Byte 0
//     pde->low_addr = (entry >> 8) & 0xFF;    // Byte 1
//     pde->high_addr = (entry >> 16) & 0xFFFF;  // Bytes 2 y 3
// }

// /**
//  * Función para "desempaquetar" la dirección física de una pde_t.
//  */
// static paddr_t get_pde_addr(pde_t *pde) {
//     // Reconstruir el valor de 32 bits desde los campos de la struct
//     uint32_t entry = pde->config | (pde->low_addr << 8) | (pde->high_addr << 16);
    
//     // Devolver solo la parte de la dirección
//     return (entry & PAGE_ADDR_MASK);
// }

// void map_page(gen_pt_t *gen_pt, vaddr_t vaddr, paddr_t paddr, uint32_t perms) {
//     pde_t *page_dir = (pde_t *) gen_pt->root;
   
//     // 1. Índice del Page Directory
//     uint32_t pdi = (vaddr >> 22) & 0x3FF;

//     // 2. Entrada del Page Directory
//     pde_t *pde_ptr = &page_dir[pdi];

//     // 3. Crear Page Table si no existe
//     if (!(pde_ptr->config & PAGE_P_PRESENT)) {
//         paddr_t new_pt_addr = alloc_pages(1);
//         memset((void *)new_pt_addr, 0, PAGE_SIZE);

//         uint32_t pde_perms = PAGE_P_PRESENT | PAGE_P_READ_WRITE | PAGE_P_USER;
//         set_pde_entry(pde_ptr, new_pt_addr, pde_perms);
//     }

//     // 4. Obtener la dirección física de la PT
//     paddr_t pt_addr = get_pde_addr(pde_ptr);
//     pte_t *page_table = (pte_t *)pt_addr;

//     // 5. Índice del Page Table
//     uint32_t pti = (vaddr >> 12) & 0x3FF;

//     // 6. Configurar la entrada PTE
//     pte_t *pte_ptr = &page_table[pti];
//     set_pte_entry(pte_ptr, paddr, perms);
// }


#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "paging.h" // Aporta page_info y page_manager

// EN TEORIA ES LO MISMO QUE EN RISCV
extern char __free_ram[], __free_ram_end[], __kernel_base[], __kernel_base_end[];

struct page_info* free_pages;
struct page_manager main_page_table;

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

void load_cr3(uint32_t pde_paddr) {
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
                struct page_info *page_to_free = page_info_to_pa(allocated_list[i]);
                page_free(page_to_free);
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
