#include "paging.h"
#include "inc/types.h"
#include "inc/common.h"
#include "arch_inc/mem_constants.h"

#define NUM_ENTRIES 1024
#define PDT_SIZE NUM_ENTRIES * sizeof(pde_t)
#define PT_SIZE  NUM_ENTRIES * sizeof(pte_t)

#define VIRTUAL_TO_PDT_IDX(a) ((a >> 20) & 0x3FF)

#define PS_4KB 0x00
#define PS_4MB 0x01

#define IS_ENTRY_PRESENT(e) ((e)->config && 0x01)

extern char __free_ram[], __free_ram_end[];

struct pdirectory* _cur_directory=0;
paddr_t	_cur_pdbr=0;

typedef uint32_t pd_entry;

bool vm_manager_switch_pdirectory (struct pdirectory* dir) {
 
	if (!dir)
		return false;
 
	_cur_directory = dir;
	load_cr3 (_cur_directory);
	return true;
}
 
struct pdirectory* vm_manager_get_directory() {
 
	return _cur_directory;
}


void vmmngr_map_page (paddr_t phys, paddr_t virt) {

    //! get page directory
    struct pdirectory* page_directory = vm_manager_get_directory();
 
    //! get page table
    pd_entry* e = &page_directory->m_entries [PAGE_DIRECTORY_INDEX ((uint32_t) virt) ];
    if ( (*e & I86_PTE_PRESENT) != I86_PTE_PRESENT) {
 
       //! page table not present, allocate it
       paddr_t pt_addr = alloc_pages (1);
       if (pt_addr == 0){
            return;
       }
       struct ptable* table = (struct ptable*) pt_addr;
       if (!table)
          return;
 
       //! clear page table
       memset (pt_addr, 0, sizeof(struct ptable));
 
       //! create a new entry
       pd_entry* entry =
          &page_directory->m_entries [PAGE_DIRECTORY_INDEX ( (uint32_t) virt) ];
 
       //! map in the table (Can also just do *entry |= 3) to enable these bits
       pd_entry_add_attrib (entry, I86_PDE_PRESENT);
       pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
       pd_entry_set_frame (entry, pt_addr);
    }
 
    //! get table
    struct ptable* table = (struct ptable*) PAGE_GET_PHYSICAL_ADDRESS ( e );
 
    //! get page
    pd_entry* page = &table->m_entries [ PAGE_TABLE_INDEX ( (uint32_t) virt) ];
 
    //! map it in (Can also do (*page |= 3 to enable..)
    pt_entry_set_frame ( page, phys);
    pt_entry_add_attrib ( page, I86_PTE_PRESENT);
 }

// extern char __kernel_base[], __kernel_base_end[], __free_ram[], __free_ram_end[], __stack_top[];

/*
In the page directory, each entry points to a page table. 
In the page table, each entry points to a 4 KiB physical page frame.

Each page directory has 1024 entries (PDEs).
Each page table also has 1024 entries (PTEs).
1024 * 1024 * 4KiB = 4GB

Address translation involves dividing the virtual address into three parts: 
    - the most significant 10 bits (bits 22-31) specify the index of the page directory entry
    - the next 10 bits (bits 12-21) specify the index of the page table entry
    - the least significant 12 bits (bits 0-11) specify the page offset
*/


/*
Purpose: CR3 serves as the Page Directory Base Register (PDBR).
Usage: It holds the physical address of the first page directory for the current task.

Physical Addressing: The upper 20 bits of CR3 are used for the physical address, 
while the lower bits may be used for process-context identifiers if the PCIDE bit in CR4 is set.
*/
extern void pdt_set(uint32_t pdt_addr); // TODO: para cr3

// Reserva espacio estático para estructuras gen_pt_t simples
static gen_pt_t pt_pool[32];
static int pt_index = 0;

gen_pt_t* get_gen_table(void) {
    // Virtual memory for the generic table
    gen_pt_t *pt = &pt_pool[pt_index++];
    memset(pt, 0, sizeof(*pt));

    // Allocate the actual physical page directory
    paddr_t pde_paddr = alloc_pages(1);
    if (!pde_paddr) PANIC("out of memory (page_dir)");
    memset((void*)pde_paddr, 0, PAGE_SIZE);

    pt->paddr = pde_paddr;
    pt->root  = (void*)pde_paddr; // identity map today

    return pt;
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


void vm_manager_free_page (pd_entry* pte_e) {}


inline pd_entry* vmmngr_ptable_lookup_entry (struct ptable* p, paddr_t addr) {
	if (p)
		return &p->m_entries[ PAGE_TABLE_INDEX (addr) ];
	return NULL;
}

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

}


void vmmngr_initialize () {

    //! allocate default page table
    paddr_t p_table = alloc_pages(1);
    paddr_t p_table2 = alloc_pages(1);

    if (p_table == 0 || p_table2 == 0){
        return;
    }

    struct ptable* table = (struct ptable*) p_table;

    //! allocates 3gb page table
    struct ptable* table2 = (struct ptable*) p_table2;
 
    //! clear page table
    memset (p_table, 0, sizeof (struct ptable));
 
    //! 1st 4mb are idenitity mapped
    for (int i=0, frame=0x0, virt=0x00000000; i<1024; i++, frame+=4096, virt+=4096) {
 
       //! create a new page
       pd_entry page=0;
       pt_entry_add_attrib (&page, I86_PTE_PRESENT);
       pt_entry_set_frame (&page, frame);
 
       //! ...and add it to the page table
       table2->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
    }
 
    //! map 1mb to 3gb (where we are at)
    for (int i=0, frame=0x100000, virt=0xc0000000; i<1024; i++, frame+=4096, virt+=4096) {
 
       //! create a new page
       pd_entry page=0;
       pt_entry_add_attrib (&page, I86_PTE_PRESENT);
       pt_entry_set_frame (&page, frame);
 
       //! ...and add it to the page table
       table->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
    }
 
    paddr_t pa_free_ram_start = (paddr_t)__free_ram;
    paddr_t pa_free_ram_end = (paddr_t)__free_ram_end;

    for (paddr_t pa = pa_free_ram_start; pa < pa_free_ram_end; pa += PAGE_SIZE) {
        // Mapeo 1:1 (virtual = físico)
        // vmmngr_map_page [cite: 119-133] asignará PTs bajo demanda
        vmmngr_map_page(pa, pa); 
    }

    //! create default directory table
    // REVISAR ALLOC PAGES PARA MAS DE UNA PAGINA
    paddr_t p_dir = alloc_pages(3);
    if (p_dir == 0){
        return;
    }

    struct pdirectory*   dir = (struct pdirectory*) p_dir;
 
   //! clear directory table and set it as current
   memset (p_dir, 0, sizeof (struct pdirectory));
 

    //CHECKPOINT

    //! get first entry in dir table and set it up to point to our table
    pd_entry* entry = &dir->m_entries [PAGE_DIRECTORY_INDEX (0xc0000000) ];
    pd_entry_add_attrib (entry, I86_PDE_PRESENT);
    pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
    pd_entry_set_frame (entry, table);
 
    pd_entry* entry2 = &dir->m_entries [PAGE_DIRECTORY_INDEX (0x00000000) ];
    pd_entry_add_attrib (entry2, I86_PDE_PRESENT);
    pd_entry_add_attrib (entry2, I86_PDE_WRITABLE);
    pd_entry_set_frame (entry2, table2);
 
    //! store current PDBR
    _cur_pdbr = (paddr_t) &dir->m_entries;
 
    //! switch to our page directory
    vm_manager_switch_pdirectory (dir);
 
    enable_paging();
 }

 void enable_paging(void) {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // bit PG (bit 31)
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

