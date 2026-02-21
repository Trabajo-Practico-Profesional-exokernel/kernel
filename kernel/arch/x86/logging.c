 
#include "arch/logging.h"
#include "stdio.h"
#include "console/debug.h"

void printTrap(const struct TrapFrame *tf) {
    printf("tf: %p\n", tf);
    printf("edi=%x  esi=%x  ebp=%x\n", tf->regs.edi, tf->regs.esi, tf->regs.ebp);
    printf("oesp=%x  ebx=%x  edx=%x\n", tf->regs.oesp, tf->regs.ebx, tf->regs.edx);
    printf("ecx=%x  eax=%x\n", tf->regs.ecx, tf->regs.eax);
    printf("eip=%x  esp=%x\n", tf->eip, tf->esp);
}

void printTrapFull(const FullTrapFrame *tf) {
	enable_debug_print();
    printf("tf: 0x%p\n", tf);
    printf("edi=0x%x  esi=0x%x  ebp=0x%x\n", tf->regs.edi, tf->regs.esi, tf->regs.ebp);
    printf("oesp=0x%x  ebx=0x%x  edx=0x%x\n", tf->regs.oesp, tf->regs.ebx, tf->regs.edx);
    printf("ecx=0x%x  eax=0x%x\n", tf->regs.ecx, tf->regs.eax);
    printf("eip=0x%x  cs=0x%x  esp=0x%x\n", tf->eip, tf->cs, tf->esp);
    printf("int_no=0x%u  err_code=0x%x\n", tf->int_no, tf->err_code);
	disable_debug_print();

}

void printProc(const struct Proc * proc){
    printf("proc id: %d, status: %d, page table CR3: %x\n", proc->pid, proc->status, (uint32_t)proc->pde_paddr);
	printTrap(&(proc->tf));
}
