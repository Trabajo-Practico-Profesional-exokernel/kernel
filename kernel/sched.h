#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H

#include "arch_inc/trapframe.h"
#include "arch/proc.h"

struct Proc * get_first_free_proc();
void sched_yield(FullTrapFrame *tf, uintptr_t pc);
void switch_proc(struct Proc * proc);

void init_sched(void);


// Ja.. ja .. ja, bueno cuando haya for y no hardcoeado se ira.
void set_proc_a(struct Proc * proc);
void set_proc_b(struct Proc * proc);


#endif