 
#include "arch/logging.h"
#include "stdio.h"
#include "console/debug.h"

void printTrap(const struct TrapFrame * tf){
	printf("ra: %x s0: %x s1: %x \n",tf->ra,tf->s0,tf->s1);
	printf("s2: %x s3: %x s4: %x \n",tf->s2,tf->s3,tf->s4);
	printf("s5: %x s6: %x s7: %x \n",tf->s5,tf->s6,tf->s7);
	printf("s8: %x s9: %x s10: %x \n",tf->s8,tf->s9,tf->s10);

    printf("s11: %x a0: %x a1: %x\n",tf->s11,tf->a0,tf->a1);
    printf("a2: %x a3: %x sp: %x \n",tf->a2, tf->a3, tf->sp);

}

void printTrapFull(const FullTrapFrame *tf){
    printf("ra: %x s0: %x s1: %x \n",tf->ra,tf->s0,tf->s1);
    printf("s2: %x s3: %x s4: %x \n",tf->s2,tf->s3,tf->s4);
    printf("s5: %x s6: %x s7: %x \n",tf->s5,tf->s6,tf->s7);
    printf("s8: %x s9: %x s10: %x \n",tf->s8,tf->s9,tf->s10);
    printf("s11: %x sp: %x \n",tf->s11,tf->sp);
    printf("gp: %x tp: %x t0: %x\n",tf->gp,tf->tp,tf->t0);
    printf("t1: %x t2: %x t3: %x\n",tf->t1,tf->t2,tf->t3);
    printf("t4: %x t5: %x t6: %x\n",tf->t4,tf->t5,tf->t6);
    printf("a0: %x a1: %x a2: %x\n",tf->a0,tf->a1,tf->a2);
    printf("a3: %x a4: %x a5: %x\n",tf->a3,tf->a4,tf->a5);
    printf("a6: %x a7: %x\n",tf->a6,tf->a7);
}


void printProc(const struct Proc * proc){
	printf("proc id: %d, status: %d pc: %x paddr stack: %x\n", proc->pid, proc->status, proc->pc, proc->user_sp_start);
	printTrap(&(proc->tf));
}
