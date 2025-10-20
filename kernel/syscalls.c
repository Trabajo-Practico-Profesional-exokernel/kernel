#include "inc/common.h"
#include "inc/syscalls.h"
#include "arch/trap_handling.h"
#include "arch/logging.h"

void putchar(char ch);

uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc){
    switch (tf->a3) {
        case SYS_PUTCHAR:
            putchar(tf->a0);
            break;
        default:
            printf("unexpected syscall a3=%x at pc: %x\n", tf->a3, pc);
            printTrapFull(tf);
    }

	return pc +4;
}