#include "inc/common.h"
#include "arch/logging.h"

void printTrap(const struct TrapFrame *tf) {
    debug_printf("tf: %p\n", tf);
    debug_printf("edi=%x  esi=%x  ebp=%x\n", tf->regs.edi, tf->regs.esi, tf->regs.ebp);
    debug_printf("oesp=%x  ebx=%x  edx=%x\n", tf->regs.oesp, tf->regs.ebx, tf->regs.edx);
    debug_printf("ecx=%x  eax=%x\n", tf->regs.ecx, tf->regs.eax);
    debug_printf("eip=%x  esp=%x\n", tf->eip, tf->esp);
}

void printTrapFull(const FullTrapFrame *tf) {
    // debug_printf("tf: %p\n", tf);
    // debug_printf("edi=%x  esi=%x  ebp=%x\n", tf->regs.edi, tf->regs.esi, tf->regs.ebp);
    // debug_printf("oesp=%x  ebx=%x  edx=%x\n", tf->regs.oesp, tf->regs.ebx, tf->regs.edx);
    // debug_printf("ecx=%x  eax=%x\n", tf->regs.ecx, tf->regs.eax);
    // debug_printf("eip=%x  esp=%x\n", tf->eip, tf->esp);

    printf("tf: %p\n", tf);
    printf("edi=%x  esi=%x  ebp=%x\n", tf->regs.edi, tf->regs.esi, tf->regs.ebp);
    printf("oesp=%x  ebx=%x  edx=%x\n", tf->regs.oesp, tf->regs.ebx, tf->regs.edx);
    printf("ecx=%x  eax=%x\n", tf->regs.ecx, tf->regs.eax);
    printf("eip=%x  esp=%x\n", tf->eip, tf->esp);
    printf("int_no=%u  err_code=%x\n", tf->int_no, tf->err_code);

}

void printProc(const struct Proc * proc){
    debug_printf("proc id: %d, status: %d, page table CR3: %x\n", proc->pid, proc->status, (uint32_t)proc->pde_paddr);
	printTrap(&(proc->tf));
}