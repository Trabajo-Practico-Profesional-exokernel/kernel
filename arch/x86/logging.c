#include "inc/common.h"
#include "arch/logging.h"

void printTrap(const struct TrapFrame * tf){
    printf("puntero tf %p\n", tf);
    printf("edi: %x\n",tf->edi);
    printf("esi: %x\n",tf->esi);
    printf("ebp: %x\n",tf->ebp);
    printf("oesp: %x\n",tf->oesp);
    printf("ebx: %x\n",tf->ebx);
    printf("edx: %x\n",tf->edx);
    printf("ecx: %x\n",tf->ecx);
    printf("eax: %x\n",tf->eax);
    printf("eip: %x\n",tf->eip);
    printf("esp: %x\n",tf->esp);
}

void printProc(const struct Proc * proc){
    printf("proc id: %d, status: %d, puntero: %p\n", proc->pid, proc->status, proc);
	printTrap(&(proc->tf));
}