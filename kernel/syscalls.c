#include "inc/common.h"
#include "inc/syscalls.h"
#include "arch/trap_handling.h"
#include "arch/logging.h"

#include "arch/stdio.h"


void syscall_putchar(FullTrapFrame *tf) {
    putchar(SYSCALL_ARG0(tf));
}


void syscall_exec(FullTrapFrame *tf) {
    int prog_ind = SYSCALL_ARG0(tf);
    printf("Should run program at ind %d \n", prog_ind);

    SET_SYSCALL_RET0(tf, 0)
}

void syscall_getchar(FullTrapFrame *tf) {
    while (1) {
        long ch = getchar();
        if (ch >= 0) {
            SET_SYSCALL_RET0(tf, ch); // change sys ret vl
            break;
        //} else {
            //printf("No char recv? %x \n", ch);
        }

        // yield or do something? do not stay doing nothing..
    }            
}


#define MAX_SYSCALLS 16

syscall_handler_t syscall_table[MAX_SYSCALLS] = {
    [SYS_PUTCHAR] = syscall_putchar,
    [SYS_GETCHAR] = syscall_getchar,
    [SYS_EXEC] = syscall_exec,
    // ... other handlers
};

uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc) {
    unsigned sysno = SYSCALL_SYSNO(tf);

    if (sysno >= MAX_SYSCALLS || syscall_table[sysno] == NULL) {
        printf("unexpected syscall a3=%x max sysno: %x at pc: %x\n", sysno, MAX_SYSCALLS, pc);
        printTrapFull(tf);
    } else {
        syscall_table[sysno](tf);
    }

    return pc + 4; // skip ecall
}