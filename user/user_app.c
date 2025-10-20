#include "inc/syscalls.h"

#define SLEEP_TIME 300000000

__attribute__((section(".user_func")))
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

__attribute__((section(".user_func")))
void putc(char ch) {
    syscall(SYS_PUTCHAR, ch, 0, 0);
}


__attribute__((section(".user_func")))
void sleep_time(int delay) {
    for (int i = 0; i < delay; i++)
        __asm__ __volatile__("nop"); // do nothing
}

__attribute__((section(".user_func")))
void main_app_a() {
    while (1){
        putc('h');
        putc('e');
        putc('l');
        putc('l');
        putc('o');
        putc(' ');
        putc('A');
        putc('\n');
        sleep_time(SLEEP_TIME);
    }
}

__attribute__((section(".user_func")))
void main_app_b(){
    while (1){
        putc('h');
        putc('e');
        putc('l');
        putc('l');
        putc('o');
        putc(' ');
        putc('B');
        putc('\n');
        sleep_time(SLEEP_TIME);
    }    
    //*((volatile int *) 0x80200000) = 0x1234; // new!
}

