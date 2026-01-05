#ifndef KERNEL_USER_PAGES_H
#define KERNEL_USER_PAGES_H

#include "inc/types.h"
#include "arch/proc.h"

bool has_no_user_free_pages(void);
size_t user_pages_count(void);
paddr_t alloc_user_page(void);
void free_user_page(paddr_t paddr);







int try_add_page_for_proc(struct Proc* proc, vaddr_t* allocated_vaddr);

int try_alloc_user_page(paddr_t * out_paddr);

vaddr_t get_vaddr_user_page(int ind);

paddr_t get_paddr_proc_page(struct Proc* proc, int ind);

#endif