#ifndef KERNEL_PROC_H
#define KERNEL_PROC_H

#include "inc/types.h"
#include "arch/proc.h"


void init_proc_pages(struct Proc* proc);
void free_proc_pages(struct Proc* proc);

void init_process_pde(struct Proc * proc);
void free_process_pde(struct Proc* proc);

void free_process(struct Proc * proc);


int load_paddr_stack_pages(struct Proc* proc, paddr_t * pages_arr);
vaddr_t copy_pages_code_segment(struct Proc* proc_src, struct Proc* proc_trg);



int try_add_heap_page_for_proc(struct Proc* proc, paddr_t* allocated_paddr);
paddr_t get_paddr_proc_heap_page(struct Proc* proc, size_t ind);


#include "meta/apps_info.h" // Includes auto generated app_info and indexs for apps  


void load_create_process_user(struct Proc * proc, const struct AppBinaryInfo * app_info, char ** argv);
void load_create_process_kernel(struct Proc * proc, uint32_t proc_entry);

// Ver antigua con el bin/codigo del prog ya en memoria.
// void create_process_user(struct Proc * proc, uint32_t proc_entry,
//     paddr_t user_space_start, paddr_t user_space_end);

#endif