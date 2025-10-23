#ifndef KERNEL_PROC_H
#define KERNEL_PROC_H

#include "inc/types.h"
#include "arch/proc.h"

void create_process(struct Proc * proc, uint32_t pc);
void create_process_user(struct Proc * proc, uint32_t proc_entry,
    paddr_t user_space_start, paddr_t user_space_end);

void load_create_process_user(struct Proc * proc, const void *image, size_t image_size);

#endif