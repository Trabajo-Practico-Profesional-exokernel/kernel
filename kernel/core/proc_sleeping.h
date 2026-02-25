#ifndef PROC_SLEEPING_H
#define PROC_SLEEPING_H
#include "types.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"

void syscall_sleep(FullTrapFrame *tf, uintptr_t pc);
void syscall_uptime(FullTrapFrame *tf, uintptr_t pc);

void reset_proc_uptime(struct Proc*  proc);
void init_proc_uptime(struct Proc*  proc);

int count_processes(void);

uint64_t get_sys_uptime(void);
#endif