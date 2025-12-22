
#define SLEEP_TIME 150000000

// __attribute__((section(".text.proc_a")))
// __attribute__((section(".text.proc_b")))

__attribute__((section(".text.proc_a.entry")))
void proc_a_entry(void) {
    printf("starting process A\n");
    while (1) {
        sleep(SLEEP_TIME);
        printf("PROC A AFTER SLEEP\n");
    }
}


__attribute__((section(".text.proc_b.entry")))
void proc_b_entry(void) {
    printf("starting process B\n");
    while (1) {
        sleep(SLEEP_TIME);
        printf("PROC B AFTER SLEEP\n");
    }
}