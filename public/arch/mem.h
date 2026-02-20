#ifndef INC_MEM_MANAGER
#define INC_MEM_MANAGER


//#include "arch_inc/mem.h"
#include "types.h"

void pde_init();
void mem_init(void);
paddr_t get_next_free_page();
paddr_t alloc_pages(uint32_t n);
int get_free_ram_memory();
// Map a page vaddr to paddr
//void map_page(gen_pt_t *table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags); //TODO: CHANGE

paddr_t get_paddr_page_ind(uint32_t ind);
paddr_t get_paddr_last_page();

paddr_t get_paddr_kernel_start();
paddr_t get_paddr_kernel_end();

paddr_t get_paddr_for(uint32_t *table1, vaddr_t vaddr);


uint32_t * init_user_pde_table(void);
uint32_t * get_kernel_pde(void);

void switch_page_table(uint32_t *table_next);
void switch_to_kernel_tables(void);



paddr_t direct_map_range(paddr_t *table1, paddr_t range_start, paddr_t range_end, uint32_t flags);
paddr_t offset_map_range(paddr_t *table1, paddr_t range_start, paddr_t range_end, uint32_t flags, vaddr_t mapped_vstart);

void direct_map_all_pages(uint32_t *table1, paddr_t start, uint32_t flags);
void map_page(uint32_t *table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags);

paddr_t get_paddr_page(uint32_t *page_table, size_t ind_pte);
paddr_t get_paddr_page_table(uint32_t *page_directory, size_t ind_pde);
void get_vaddr_indexs(vaddr_t vaddr, size_t* ind_pde, size_t* ind_pte);


// start and end will go to the prev indexs... if not aligned with page size... i.e ignoring offset in page
// Macro does walk pde table from vaddr start to vaddr end ... exposing as it seen
// _paddr == paddr of virtual page
// _curr_pde == curr pde entry index
// _curr_pte == curr pte entry index
// _end_pde == final pde entry index
// _end_pte == final pte entry index
// _pt_table == current pte_table paddr
#define WALK_MEM_PAGES(                                              \
        pde_paddr, start, end,                                       \
        ON_MISSING_PDE,                                              \
        ON_MISSING_PTE,                                              \
        BODY                                                         \
)                                                                    \
do {                                                                 \
    size_t _curr_pde, _curr_pte, _end_pde, _end_pte;                 \
                                                                     \
    get_vaddr_indexs((start), &_curr_pde, &_curr_pte);               \
    get_vaddr_indexs((end),   &_end_pde, &_end_pte);                 \
                                                                     \
    uint32_t *_pte_table = (uint32_t*)                               \
        get_paddr_page_table((uint32_t *)(pde_paddr), _curr_pde);    \
                                                                     \
    for (;;) {                                                       \
        if (!_pte_table) {                                           \
            ON_MISSING_PDE;                                          \
        }                                                            \
                                                                     \
        uint32_t _paddr =                                            \
            get_paddr_page(_pte_table, _curr_pte);                   \
                                                                     \
        if (!_paddr) {                                               \
            ON_MISSING_PTE;                                          \
        }                                                            \
                                                                     \
        BODY                                                         \
                                                                     \
        _curr_pte++;                                                 \
                                                                     \
        if (_curr_pte == 1024) {                                     \
            _curr_pte = 0;                                           \
            _curr_pde++;                                             \
            _pte_table = (uint32_t*)                                 \
                get_paddr_page_table(                                \
                    (uint32_t *)(pde_paddr), _curr_pde);             \
        }                                                            \
                                                                     \
        if (_curr_pde > _end_pde ||                                  \
            (_curr_pde == _end_pde && _curr_pte >= _end_pte))        \
            break;                                                   \
    }                                                                \
} while (0)


#endif /* !*/
