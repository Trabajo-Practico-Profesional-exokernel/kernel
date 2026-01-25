 
#include "arch/trap_handling.h"
#include "arch/logging.h"
#include "string.h"
#include "stdlib.h"
#include "arch/stdio.h"
#include "arch/trap.h"
#include "sched.h"
#include "stdio.h"
#include "console/debug.h"

void syscall_putchar(FullTrapFrame *tf, uintptr_t pc) {
    putchar(SYSCALL_ARG0(tf));
}



void syscall_getchar(FullTrapFrame *tf, uintptr_t pc) {
    while (1) {
        long ch = getchar();
        if (ch >= 0) {
            SET_SYSCALL_RET0(tf, ch); // change sys ret vl
            break;
        }

        // save_curr_proc_state(tf, pc);
        // get_curr()->status = PROC_NOT_RUNNABLE;
        // sched_yield();
    }            
}

void syscall_uptime(FullTrapFrame *tf, uintptr_t pc){
    SET_SYSCALL_RET0(tf, ticks);
}

#define MAX_SYSCALLS 50
syscall_handler_t syscall_table[MAX_SYSCALLS] = {
    [SYS_PUTCHAR] = syscall_putchar,
    [SYS_GETCHAR] = syscall_getchar,
    [SYS_UPTIME] = syscall_uptime,
    // ... other handlers, wil be registered with register_syscall
};
void register_syscall(size_t sysno, syscall_handler_t handler){
    syscall_table[sysno] = handler;
}

uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc) {
    unsigned sysno = SYSCALL_SYSNO(tf);

    if (sysno >= MAX_SYSCALLS || syscall_table[sysno] == NULL) {
        debug_printf("unexpected syscall a3=%x max sysno: %x at pc: %x\n", sysno, MAX_SYSCALLS, pc);
        printTrapFull(tf);
    } else {
        // #if IS_RISC
        // #else
        // debug_printf("Asked to exec syscall sysno=%u max sysno: %x at pc: %x\n", sysno, MAX_SYSCALLS, pc);
        // #endif
        syscall_table[sysno](tf, pc);
    }

    return pc + 4; // skip ecall
}
