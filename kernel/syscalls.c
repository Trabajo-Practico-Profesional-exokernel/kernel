#include "inc/common.h"
#include "inc/syscalls.h"
#include "arch/trap_handling.h"
#include "arch/logging.h"

#include "arch/stdio.h"

uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc){
    switch (SYSCALL_SYSNO(tf)) {
        case SYS_PUTCHAR:
            putchar(SYSCALL_ARG0(tf));
            break;
        case SYS_GETCHAR:
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
            break;
        default:
            printf("unexpected syscall a3=%x at pc: %x\n", tf->a3, pc);
            printTrapFull(tf);
    }

	return pc +4;
}