#include "inc/common.h"
#include "arch/logging.h"


void printTrap(const struct TrapFrame * tf){
	printf("ra: %x s0: %x s1: %x \n",tf->ra,tf->s0,tf->s1);
	printf("s2: %x s3: %x s4: %x \n",tf->s2,tf->s3,tf->s4);
	printf("s5: %x s6: %x s7: %x \n",tf->s5,tf->s6,tf->s7);
	printf("s8: %x s9: %x s10: %x \n",tf->s8,tf->s9,tf->s10);
	printf("s11: %x sp: %x \n",tf->s11,tf->sp);
}

void printProc(const struct Proc * proc){
	printf("proc id: %d, status: %d\n", proc->pid, proc->status);
	printTrap(&(proc->tf));
}
