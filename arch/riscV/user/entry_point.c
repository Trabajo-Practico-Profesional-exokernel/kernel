#include "lib.h"

// extern char __stack_top[];

extern void main(void);

#define VADDR_USER_STACK_SIZE 2* 4096 // 8kb
#define VADDR_USER_STACK_BASE 0x1800000 + VADDR_USER_STACK_SIZE
#define VADDR_USER_STACK_TOP VADDR_USER_STACK_BASE

__attribute__((noreturn)) void do_exit(void) {
    printf("-------> PROCESS EXITED NORMALLY!\n");
    exit(0); // Syscall exit!
}

//__attribute__((section(".user_func")))
int syscall(int sysno, int arg0, int arg1, int arg2) {
    register int a0 __asm__("a0") = arg0;
    register int a1 __asm__("a1") = arg1;
    register int a2 __asm__("a2") = arg2;
    register int a3 __asm__("a3") = sysno;

    __asm__ __volatile__("ecall"
                         : "=r"(a0)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3)
                         : "memory");

    return a0;
}

void sleep(int delay) {
    for (int i = 0; i < delay; i++)
        __asm__ __volatile__("nop"); // do nothing
}


void arg_main(int argc, char** argv){
    printf("Prog got argc: %d and argv: %x\n", argc, argv);
    int curr= 0;
    for(curr = 0; curr < argc; curr++) {
        printf("Prog got arg pointer argv[%d]: %x ", curr, argv[curr]);    
        printf("=> '%s'\n", argv[curr]);    
    }
    main();
    
    do_exit();
}
__attribute__((section(".text.start")))
__attribute__((naked))
void start() {
    __asm__ __volatile__(
        "call arg_main           \n"
        ::
    );
}