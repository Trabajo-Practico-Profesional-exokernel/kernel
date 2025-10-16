#ifndef INC_SWITCH
#define INC_SWITCH

#include "arch/proc.h"
#include "arch_inc/trapframe.h"

__attribute__((naked)) 
void switch_context(struct Proc* next);

void sleep(int delay); // Just to able to sleep basically


void update_trapframe(struct Proc * proc, FullTrapFrame *tf);
void init_trapframe(struct Proc * proc, uint32_t init_ins);

#endif /* !*/
