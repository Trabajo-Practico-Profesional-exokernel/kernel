
#define SLEEP_TIME 150000000

// extern int syscall(int num, int a, int b, int c);
#ifdef IS_RISC
#else
__attribute__((section(".text.proc_a")))
int syscall(int num, int a, int b, int c)
{
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(a), "c"(b), "d"(c)
        : "memory"
    );
    return ret;
}
#endif

// __attribute__((section(".text.proc_a")))
// __attribute__((section(".text.proc_b")))


__attribute__((section(".text.proc_a.entry")))
void proc_a_entry(void) {
    // printf("starting process A\n");
    syscall(0,0,0,0);
    // while (1) {
    //     // sleep(SLEEP_TIME);
    //     // printf("PROC A AFTER SLEEP\n");
    // }
}


__attribute__((section(".text.proc_b.entry")))
void proc_b_entry(void) {
    // printf("starting process B\n");
    while (1) {
        // sleep(SLEEP_TIME);
        // printf("PROC B AFTER SLEEP\n");
    }
}