#include "arch/switch.h"

__attribute__((naked)) 
void switch_context(struct Proc* next) {}

void sleep(int delay) {}

void init_trapframe(struct Proc * proc, uint32_t init_ins){}

void update_trapframe(struct Proc *proc, FullTrapFrame *tf){}