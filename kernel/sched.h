#include "arch_inc/trapframe.h"

void init_sched(void);
void sched_yield(FullTrapFrame *tf);