#ifndef INC_TRAP_HANDLING
#define INC_TRAP_HANDLING

//#include "arch_inc/trapframe.h"

// Receives sys num and current pc... returns which ins to go back
int handle_syscall(int sys_num, int pc);

// For now hardcoded on trap.c on arch implementation. Might be cleaner to have it be
//void handle_clock_interrupt(struct FullTrapFrame *f);


#endif /* !*/
