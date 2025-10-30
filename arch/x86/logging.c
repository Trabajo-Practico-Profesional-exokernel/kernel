#include "inc/common.h"
#include "arch/logging.h"

void printTrap(const struct TrapFrame *tf) {
    printf("tf: %p\n", tf);
    printf("edi=%x  esi=%x  ebp=%x\n", tf->edi, tf->esi, tf->ebp);
    printf("oesp=%x  ebx=%x  edx=%x\n", tf->oesp, tf->ebx, tf->edx);
    printf("ecx=%x  eax=%x\n", tf->ecx, tf->eax);
    printf("eip=%x  esp=%x\n", tf->eip, tf->esp);
}

void printTrapFull(const FullTrapFrame *tf) {
    printf("tf: %p\n", tf);
    printf("edi=%x  esi=%x  ebp=%x\n", tf->edi, tf->esi, tf->ebp);
    printf("oesp=%x  ebx=%x  edx=%x\n", tf->oesp, tf->ebx, tf->edx);
    printf("ecx=%x  eax=%x\n", tf->ecx, tf->eax);
    printf("eip=%x  es=%x\n", tf->eip, tf->es);
}

void printProc(const struct Proc * proc){
    printf("proc id: %d, status: %d, puntero: %p\n, page table: %p\n", proc->pid, proc->status, proc, proc->page_table->paddr);
	printTrap(&(proc->tf));
}