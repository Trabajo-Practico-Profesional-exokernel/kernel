#ifndef INC_TRAP_RISCV
#define INC_TRAP_RISCV
#include "arch_inc/trapframe.h"
// Declara los metodos que si no no estarian declarados.
__attribute__((naked))
__attribute__((aligned(4)))
void trap_entry(void);

void handle_trap(struct FullTrapFrame *f);


#endif /* !*/
