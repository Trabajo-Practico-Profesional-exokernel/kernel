#include "inc/common.h"
#include "inc/syscalls.h"
#include "arch/trap_handling.h"
#include "arch/logging.h"

#include "arch/stdio.h"

uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc){
    switch (tf->a3) {
        case SYS_PUTCHAR:
            putchar(tf->a0);
            break;
        case SYS_GETCHAR:
            while (1) {
                long ch = getchar();
                if (ch >= 0) {
                    tf->a0 = ch; // change a0 the restored value
                    break;
                } else {
                    printf("No char recv? %x \n", ch);
                }
            }            
            break;
        default:
            printf("unexpected syscall a3=%x at pc: %x\n", tf->a3, pc);
            printTrapFull(tf);
    }

	return pc +4;
}