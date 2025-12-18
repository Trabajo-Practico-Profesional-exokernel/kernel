#ifndef INC_TRAP_HANDLING
#define INC_TRAP_HANDLING

#include "inc/types.h"
#include "inc/syscalls.h" // Why not... usually you want it too.
#include "arch_inc/trapframe.h"

// Receives sys num and current pc... returns which ins to go back
uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc);

// Some better ways than just a switch with sysno!
typedef void (*syscall_handler_t)(FullTrapFrame *tf, uintptr_t pc);

void register_syscall(size_t sysno, syscall_handler_t handler);


// General default group of syscalls to be registered ... distributed in different files
// To avoid having all syscall handler centralized in a file making it hard to understand
void init_syscalls_fs(void);
void init_syscalls_ipc(void);
void init_syscalls_proc(void);

#endif /* !*/
