#ifndef KERNEL_PROC_H
#define KERNEL_PROC_H

#include "types.h"
#include "arch/proc.h"


void init_process_pde(struct Proc * proc);
void free_process_pde(struct Proc* proc);

void free_process(struct Proc * proc);


#include "proc_disk_loading.h"

void load_create_process_user(struct Proc * proc, const struct BinaryAppEntry * app_info, char ** argv);

int reload_process_user(struct Proc * proc, const struct BinaryAppEntry * app_info, char ** argv);

void load_create_process_kernel(struct Proc * proc, uint32_t proc_entry);

int load_create_forked(struct Proc* parent, struct Proc* child);

// Ver antigua con el bin/codigo del prog ya en memoria.
// void create_process_user(struct Proc * proc, uint32_t proc_entry,
//     paddr_t user_space_start, paddr_t user_space_end);

#endif