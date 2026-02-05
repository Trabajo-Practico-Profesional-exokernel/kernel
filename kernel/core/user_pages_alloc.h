#ifndef KERNEL_USER_PAGES_H
#define KERNEL_USER_PAGES_H

#include "types.h"
#include "arch/proc.h"

bool has_no_user_free_pages(void);
size_t user_pages_count(void);
paddr_t alloc_user_page(void);
size_t get_free_user_memory(void);

void free_user_page(paddr_t paddr);

int try_alloc_user_page(paddr_t * out_paddr);

#endif
