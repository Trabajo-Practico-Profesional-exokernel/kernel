

#include "lib.h"
#include "stdio.h"
#include "console/debug.h"

extern void main(int argc, char** argv)__attribute__((weak));

__attribute__((noreturn)) void do_exit(void) {
    VERBOSE_DEBUG_PRINTF("-------> PROCESS EXITED NORMALLY!\n");
    exit(0); // Syscall exit!
}

int syscall(int sysno, int arg0, int arg1, int arg2, int arg3) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(sysno), "b"(arg0), "c"(arg1), "d"(arg2), "S"(arg3)
        : "memory"
    );
    return ret;
}


void sleep(int delay) {
    for (int i = 0; i < delay; i++)
        __asm__ __volatile__("nop"); // do nothing
}

extern void init_environ(char ** envp);

// __attribute__((section(".text.start")))
void _start(int argc, char** argv,char** envp){
    init_environ(envp);
    VERBOSE_DEBUG_PRINTF("Prog got argc: %d and argv: %x\n", argc, argv);

    if(argc > 0){
        VERBOSE_DEBUG_PRINTF("GOT ARGS:\n");
        int curr= 0;
        for(curr = 0; curr < argc; curr++) {
            VERBOSE_DEBUG_PRINTF("Prog got arg pointer argv[%d]: %x ", curr, argv[curr]);    
            VERBOSE_DEBUG_PRINTF("=> '%s'\n", argv[curr]);    
        }
    }
    
    main(argc, argv);
    
    do_exit();
}

__attribute__((naked, section(".text.start")))
void _entry(void) {
    __asm__ volatile (
        /* Align stack to 16 bytes */
        "andl $~0xF, %esp        \n"

        /* Push C ABI arguments (right to left) */
        "pushl %ecx             \n" /* envp */
        "pushl %ebx             \n" /* argv */
        "pushl %eax             \n" /* argc */

        /* Call C entry */
        "call _start            \n"

        /* If _start returns, hang */
        "hlt                    \n"
        "jmp .                  \n"
    );
}