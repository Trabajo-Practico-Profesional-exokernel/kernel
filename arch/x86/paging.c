#include "paging.h"
#include "inc/types.h"
#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "std/string.h"

#define NUM_ENTRIES 1024

extern char __free_ram[], __free_ram_end[];

// global vars
static struct pdirectory* _cur_directory=0;
static paddr_t _cur_pdbr = 0; // TODO: check if change to kernel_pdir



// Loads new Page Directory to CR3 and to global variable
bool vm_manager_switch_pdirectory(paddr_t p_dir_phys) {
    if (!p_dir_phys)
        return false;

    _cur_pdbr = p_dir_phys;
    _cur_directory = (struct pdirectory*) p_dir_phys; 
    
    switch_page_table(_cur_pdbr);
    return true;
}
 
struct pdirectory* vm_manager_get_directory() {
 
	return _cur_directory;
}


void vmmngr_map_page (paddr_t phys, paddr_t virt) {

    //! get page directory
    struct pdirectory* page_directory = vm_manager_get_directory();
 
    //! get page table (from PDE)
    pd_entry* e = &page_directory->m_entries [PAGE_DIRECTORY_INDEX ((uint32_t) virt) ];
    if ( (*e & I86_PTE_PRESENT) != I86_PTE_PRESENT) {
 
       //! page table not present, allocate it
       paddr_t pt_addr = alloc_pages (1);
       if (pt_addr == 0){
            PANIC("vmmngr_map_page: out of memory alloc_pages");
       }
 
       //! clear page table
       memset ((void*)pt_addr, 0, sizeof(struct ptable));
 
       //! create a new entry
       pd_entry* entry =
          &page_directory->m_entries [PAGE_DIRECTORY_INDEX ( (uint32_t) virt) ];
 
       //! configure permissions and set PDE to point to PT
       pd_entry_add_attrib (entry, I86_PDE_PRESENT);
       pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
    //    pd_entry_add_attrib (entry, I86_PDE_USER); // TODO: SET PERMISSIONS
       pd_entry_set_frame (entry, pt_addr);
    }
 
    //! get table
    struct ptable* table = (struct ptable*) PAGE_GET_PHYSICAL_ADDRESS ( *e );
 
    //! get page
    pd_entry* page = &table->m_entries [ PAGE_TABLE_INDEX ( (uint32_t) virt) ];
 
    //! map it in (Can also do (*page |= 3 to enable..)
    pt_entry_set_frame ( page, phys);
    pt_entry_add_attrib ( page, I86_PTE_PRESENT);
    pt_entry_add_attrib(page, I86_PTE_WRITABLE); // ADDED
    pt_entry_add_attrib(page, I86_PTE_USER); //ADDED. TODO: Debería heredar los permisos de USER si es necesario
 }

bool vm_manager_alloc_page (pd_entry* pte_e) {
 
	//! allocate a free physical frame
	paddr_t pde_paddr = alloc_pages(1);
	if (pde_paddr == 0) {
        return false;
    }
 
	//! map it to the page
	pt_entry_set_frame (pte_e, pde_paddr);
	pt_entry_add_attrib (pte_e, I86_PTE_PRESENT);
	return true;
}


void vm_manager_free_page (pd_entry* pte_e) {
    //TODO:
}


// inline pd_entry* vmmngr_ptable_lookup_entry (struct ptable* p, paddr_t addr) {
// 	if (p)
// 		return &p->m_entries[ PAGE_TABLE_INDEX (addr) ];
// 	return NULL;
// }

inline void pt_entry_add_attrib (pd_entry* e, uint32_t attrib) {
	*e |= attrib;
}

inline void pt_entry_del_attrib (pd_entry* e, uint32_t attrib) {
	*e &= ~attrib;
}

inline void pt_entry_set_frame (pd_entry* e, paddr_t addr) {
	*e = (*e & ~I86_PTE_FRAME) | addr;
}

inline bool pt_entry_is_present (pd_entry e) {
	return e & I86_PTE_PRESENT;
}

inline bool pt_entry_is_writable (pd_entry e) {
	return e & I86_PTE_WRITABLE;
}

inline paddr_t pt_entry_pfn (pd_entry e) {
	return e & I86_PTE_FRAME;
}

inline void pd_entry_add_attrib (pd_entry* e, uint32_t attrib) {
	*e |= attrib;
}

inline void pd_entry_del_attrib (pd_entry* e, uint32_t attrib) {
	*e &= ~attrib;
}

inline void pd_entry_set_frame (pd_entry* e, paddr_t addr) {
	*e = (*e & ~I86_PDE_FRAME) | addr;
}

inline bool pd_entry_is_present (pd_entry e) {
	return e & I86_PDE_PRESENT;
}

inline bool pd_entry_is_writable (pd_entry e) {
	return e & I86_PDE_WRITABLE;
}

inline paddr_t pd_entry_pfn (pd_entry e) {
	return e & I86_PDE_FRAME;
}

inline bool pd_entry_is_user (pd_entry e) {
	return e & I86_PDE_USER;
}

inline bool pd_entry_is_4mb (pd_entry e) {
	return e & I86_PDE_4MB;
}

inline void pd_entry_enable_global (pd_entry e) {
    //TODO:
}


// El mapeo de identidad (identity mapping) es una configuración de paginación
// donde las direcciones virtuales se mapean a las mismas direcciones físicas.
// Cuando tu kernel habilita la paginación (al setear el bit PG en CR0), 
// la CPU está ejecutando código que reside en direcciones físicas bajas.
// Si no se hace el identity map 1:1 hay triple fault (obligatorio en x86).

// Incializa el manejador de memoria virtual
// TODO: chequear. estamos mapeando va==pa. podriamos robar KADDR de JOS
void vmmngr_initialize() {

    //! allocate default page table
    paddr_t p_table = alloc_pages(1); // Para 0xC0000000 (Kernel)
    paddr_t p_table2 = alloc_pages(1); // Para 0x00000000 (Identity Map)

    if (p_table == 0 || p_table2 == 0){
        PANIC("vmmngr_initialize: out of memory for PTs");
    }

    struct ptable* table = (struct ptable*) p_table;
    struct ptable* table2 = (struct ptable*) p_table2;  //3gb page table
 
    memset(table, 0, sizeof (struct ptable));
    memset(table2, 0, sizeof(struct ptable));
 
    //! First 4MB are idenitity mapped (kernel code)
    // identity map: va == pa
    for (int i=0, frame=0x0, virt=0x00000000; i<1024; i++, frame+=PAGE_SIZE, virt+=PAGE_SIZE) {
 
       //! create a new page
       pd_entry page=0;
       pt_entry_add_attrib (&page, I86_PTE_PRESENT); 
       pt_entry_add_attrib(&page, I86_PTE_WRITABLE); //ADDED
       pt_entry_set_frame (&page, frame);
       table2->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
    }
 
    //! map 1mb to 3gb (where we are at)
    // El kernel se ejecuta virtualmente en 3 GB, pero físicamente vive desde 1 MB
    for (int i=0, frame=0x100000, virt=VADDR_KERNEL_BASE; i<1024; i++, frame+=PAGE_SIZE, virt+=PAGE_SIZE) {
 
       if (frame >= 0x500000) break; // ADDED: Limitar a 4MB de kernel por ahora

       //! create a new page
       pd_entry page=0;
       pt_entry_add_attrib (&page, I86_PTE_PRESENT);
       pt_entry_add_attrib(&page, I86_PTE_WRITABLE); //ADDED
       pt_entry_set_frame (&page, frame);
       table->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
    }
 
    paddr_t pa_free_ram_start = (paddr_t)__free_ram;
    paddr_t pa_free_ram_end = (paddr_t)__free_ram_end;

    // La RAM libre queda identity-mapped
    for (paddr_t pa = pa_free_ram_start; pa < pa_free_ram_end; pa += PAGE_SIZE) {
        // Mapeo 1:1 (virtual = físico)
        // vmmngr_map_page [cite: 119-133] asignará PTs bajo demanda
        vmmngr_map_page(pa, pa); 
    }

    //! create default directory table
    // TODO: CHECK cambie de alloc_pages(3) a 1
    paddr_t p_dir = alloc_pages(1);
    if (p_dir == 0){
        PANIC("vmmngr_initialize: out of memory for PD");
    }

    struct pdirectory*   dir = (struct pdirectory*) p_dir;
    memset (dir, 0, sizeof (struct pdirectory));
 
    //! get first entry in dir table and set it up to point to our table
    pd_entry* entry = &dir->m_entries [PAGE_DIRECTORY_INDEX (VADDR_KERNEL_BASE) ];
    pd_entry_add_attrib (entry, I86_PDE_PRESENT);
    pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
    pd_entry_set_frame (entry, p_table);
 
    pd_entry* entry2 = &dir->m_entries [PAGE_DIRECTORY_INDEX (0x00000000) ];
    pd_entry_add_attrib (entry2, I86_PDE_PRESENT);
    pd_entry_add_attrib (entry2, I86_PDE_WRITABLE);
    pd_entry_set_frame (entry2, p_table2);
 
    //! store current PDBR
    _cur_pdbr = (paddr_t) &dir->m_entries;
 
    //! switch to our page directory
    vm_manager_switch_pdirectory (p_dir);
 
    enable_paging();
 }

 void enable_paging(void) {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // bit PG (bit 31)
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
    printf("PG=1\n");
}

// Mapea una pagina virtual a una fisica DENTRO de un Page Dir especifico.
void map_page(struct pdirectory* dir, vaddr_t virt, paddr_t phys, uint32_t flags) {
    pd_entry* e = &dir->m_entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

    if ((*e & I86_PDE_PRESENT) != I86_PDE_PRESENT) {
        paddr_t pt_addr = alloc_pages(1);
        if (pt_addr == 0) {
            PANIC("map_page: out of memory alloc_pages");
        }

        memset((void*)pt_addr, 0, sizeof(struct ptable));

        // Configurar la PDE para que apunte a la nueva PT
        // Asegurarse de que el usuario pueda acceder si los flags lo indican
        uint32_t pde_flags = I86_PDE_PRESENT | I86_PDE_WRITABLE;
        if (flags & I86_PTE_USER) {
            pde_flags |= I86_PDE_USER;
        }
        pd_entry_add_attrib(e, pde_flags);
        pd_entry_set_frame(e, pt_addr);
    }

    struct ptable* table = (struct ptable*) PAGE_GET_PHYSICAL_ADDRESS(*e);
    pd_entry* page = &table->m_entries[PAGE_TABLE_INDEX((uint32_t)virt)];

    pt_entry_set_frame(page, phys);
    pt_entry_add_attrib(page, flags);
}


// void map_page(struct pdirectory* dir, paddr_t phys, vaddr_t virt, uint32_t flags) {
//     uint32_t pde_index = PAGE_DIRECTORY_INDEX(virt);
//     pd_entry* pde = &dir->m_entries[pde_index];

//     if (!(*pde & I86_PDE_PRESENT)) {
//         paddr_t pt_addr = alloc_pages(1);

//         memset(KADDR(pt_addr), 0, sizeof(struct ptable));

//         uint32_t pde_flags = I86_PDE_PRESENT | I86_PDE_WRITABLE;
//         if (flags & I86_PTE_USER) pde_flags |= I86_PDE_USER;

//         pd_entry_set_frame(pde, pt_addr);
//         pd_entry_add_attrib(pde, pde_flags);
//     }

//     struct ptable* table = (struct ptable*) KADDR(PAGE_GET_PHYSICAL_ADDRESS(pde));

//     pd_entry* pte = &table->m_entries[PAGE_TABLE_INDEX(virt)];

//     pt_entry_set_frame(pte, phys);
//     pt_entry_add_attrib(pte, flags);
// }