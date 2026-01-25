
#ifndef SIMPLE_LOGGINGS
#define SIMPLE_LOGGINGS
#include "types.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"

void printTrapFull(const FullTrapFrame *tf);
void printTrap(const struct TrapFrame * tf);
void printProc(const struct Proc * proc);


#endif /* !SIMPLE LOGGINGS*/
