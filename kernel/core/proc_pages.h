#ifndef PROC_PAGES_H
#define PROC_PAGES_H

#include "types.h"
#include "arch/proc.h"

struct UserProcPages {
    paddr_t vaddr_proc_heap_end;
    // paddr_t vaddr_proc_code_end; // PODRIAN SER DINAMICOS! En vez de estaticos!
    // paddr_t vaddr_proc_stack_end;
};

int try_add_heap_page_for_proc(struct Proc* proc, paddr_t* allocated_paddr);
paddr_t get_paddr_proc_heap_page(struct Proc* proc, size_t ind);
vaddr_t get_vaddr_user_heap_page(size_t ind);


int load_paddr_stack_pages(struct Proc* proc, paddr_t * pages_arr);

void init_proc_pages(struct Proc* proc);
void init_proc_stack(struct Proc* proc);

void free_proc_pages(struct Proc* proc);
void reset_proc_heap(struct Proc* proc);

void reset_proc_range(struct Proc* proc, vaddr_t start, vaddr_t end);
int copy_mem_pages(struct Proc* src_proc, struct Proc* trg_proc);


#endif