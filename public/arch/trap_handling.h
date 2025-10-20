#ifndef INC_TRAP_HANDLING
#define INC_TRAP_HANDLING

#include "inc/types.h"
#include "arch_inc/trapframe.h"

// Receives sys num and current pc... returns which ins to go back
uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc);

// Some better ways than just a switch with sysno!
typedef void (*syscall_handler_t)(FullTrapFrame *tf);




// For now hardcoded on trap.c on arch implementation. Might be cleaner to have it be
//void handle_clock_interrupt(FullTrapFrame *f);


#endif /* !*/
