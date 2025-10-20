#include "inc/common.h"
#include "inc/syscalls.h"
#include "arch/trap_handling.h"

void putchar(char ch);


uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc){
    switch (tf->a3) {
        case SYS_PUTCHAR:
            putchar(tf->a0);
            break;
        default:
            printf("unexpected syscall a3=%x\n", tf->a3);
    }

	return pc +4;
}