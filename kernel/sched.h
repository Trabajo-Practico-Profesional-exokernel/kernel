#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H

#include "arch_inc/trapframe.h"
#include "arch/proc.h"
extern int filesystem_PID;
struct Proc * get_first_free_proc();
struct Proc * get_proc(procid_t proc_pid);

void clock_yield(FullTrapFrame *tf, uintptr_t proc_pc);

void save_curr_proc_state(FullTrapFrame *tf, uintptr_t proc_pc);
void sched_yield(void);


void switch_proc(struct Proc * proc);

void init_cpu(int cpunum);

void init_sched(void);


// Ja.. ja .. ja, bueno cuando haya for y no hardcoeado se ira.
void set_proc_a(struct Proc * proc);
void set_proc_b(struct Proc * proc);

// Tampoco deberia usarse!
void set_curr(struct Proc * proc);
struct Proc * get_curr();
struct Proc * get_proc_by_pid(int receiver_pid);

#endif