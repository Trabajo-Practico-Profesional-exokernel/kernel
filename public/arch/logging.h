
#ifndef SIMPLE_LOGGINGS
#define SIMPLE_LOGGINGS
#include "inc/types.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"


void printTrap(const struct TrapFrame * tf);
void printProc(const struct Proc * proc);


#endif /* !SIMPLE LOGGINGS*/
