#ifndef PAGING_H
#define PAGING_H

#include "inc/types.h"
#include "arch/mem.h"


struct page_info {
    uint8_t index;
    uint8_t ref;
    struct page_info* next_free_page;
};

struct page_manager {
    struct page_info* free_page_list;
    struct page_info* pages_array;
    uint32_t free_pages;
};

extern struct page_manager main_page_table;


/*
Bits  | Field
------+----------------------------------------
0–7   | config       -> flags (P, RW, US, PWT, etc.)
8–15  | low_addr     -> bits 12–19 of addr (stored shifted)
16–31 | high_addr    -> bits 20–31 of addr
*/

/* pde: page directory entry points to various pte's*/
struct pde {
    uint8_t config;
    uint8_t low_addr; /* only the highest 4 bits are used */
    uint16_t high_addr;
} __attribute__((packed));
typedef struct pde pde_t;

/* pte: page table entry points to 4 KiB blocks of physical memory */
struct pte {
    uint8_t config;
    uint8_t middle; /* only the highest 4 bits and the lowest bit are used */
    uint16_t high_addr;
    } __attribute__((packed));
typedef struct pte pte_t;

gen_pt_t* get_gen_table();

//void paging_init(uint32_t boot_page_directory);

#endif /* PAGING_H */
