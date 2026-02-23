#ifndef KERNEL_PROC_H
#define KERNEL_PROC_H

#include "types.h"
#include "arch/proc.h"


void init_process_pde(struct Proc * proc);
void free_process_pde(struct Proc* proc);

void free_process(struct Proc * proc);


#include "proc_disk_loading.h"

int load_create_process_user(struct Proc * proc, const struct BinaryAppEntry * app_info);
int reload_process_user(struct Proc * proc, const struct BinaryAppEntry * app_info);

int load_create_process_kernel(struct Proc * proc, uint32_t proc_entry);

int load_create_forked(struct Proc* parent, struct Proc* child);

int copy_to_stack_list(paddr_t* item_pointers, 
        char ** list, paddr_t* max_addr, paddr_t min_addr,vaddr_t max_vaddr);

int copy_param_pointers_from_user(struct Proc * proc, paddr_t* param_pointers, paddr_t paddr_pointers);


int init_parameters_for_proc(struct Proc * proc, char ** argv, char ** envp);

struct Proc * create_process_from_ind(int ind, char ** argv, char ** envp);
struct Proc * create_process(char* proc_name, char ** argv, char ** envp);

// Ver antigua con el bin/codigo del prog ya en memoria.
// void create_process_user(struct Proc * proc, uint32_t proc_entry,
//     paddr_t user_space_start, paddr_t user_space_end);

#endif