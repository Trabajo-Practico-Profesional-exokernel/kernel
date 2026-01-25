#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H

#include "arch_inc/trapframe.h"
#include "arch/proc.h"
extern int filesystem_PID;
extern volatile uint64_t ticks;


struct Proc * get_first_free_proc();
struct Proc * get_proc(procid_t proc_pid);

void clock_yield(FullTrapFrame *tf, uintptr_t proc_pc);

void save_curr_proc_state(FullTrapFrame *tf, uintptr_t proc_pc);
void sched_yield(void);


void switch_proc(struct Proc * proc);

void init_cpu(int cpunum);

void init_sched(void);

struct Proc * get_curr();
struct Proc * get_proc_by_pid(int receiver_pid);

#endif