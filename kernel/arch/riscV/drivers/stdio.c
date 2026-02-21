
#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "types.h"
 
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "drivers/opensbi.h"
#include "arch/mem.h"

#include "arch_inc/trap_constants.h"
#include "arch_inc/cpu.h"

#define CPU_STACK_SIZE 4096 * CPU_STACK_PAGES  
#define TRAP_STACK_SIZE 4096 * CPU_TRAP_STACK_PAGES

extern char __bss[], __bss_end[], __stack_base[], __trap_stack_base[];

void init_arch(void){
    // bss supposed to be 0s but just in case
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);    
}

#define SBI_PUTCHAR 1
#define SBI_GETCHAR 2
struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, SBI_PUTCHAR);
}

long getchar(void) {
    struct sbiret ret = sbi_call(0, 0, 0, 0, 0, 0, 0, SBI_GETCHAR);
    return ret.error;
}

#define SBI_EXT_HSM            0x48534D
#define SBI_HSM_HART_START    0
#define SBI_HSM_HART_STOP     1
#define SBI_HSM_HART_STATUS   2

static inline long sbi_hart_start(
    unsigned long hartid,
    unsigned long start_addr,
    unsigned long opaque)
{
    struct sbiret ret = sbi_call(
        hartid,          // a0
        start_addr,      // a1
        opaque,          // a2
        0, 0, 0,
        SBI_HSM_HART_START,
        SBI_EXT_HSM
    );
    return ret.error;
}



void clear(void){}


void move_cursor(uint16_t pos){
  (void)pos;
}


int kmain();
int secondary_cpu_main();





// For others cpus.. about the same tbh
__attribute__((naked))
void secondary_entry(void)
{
    __asm__ __volatile__(
        "mv tp, a0\n"                // tp = hartid

        // sp = __stack_base + (hartid + 1) * CPU_STACK_SIZE
        "la   t0, __stack_base\n"
        "li   t1, %0\n"
        "addi t2, tp, 1\n"
        "mul  t2, t2, t1\n"
        "add  sp, t0, t2\n"

        // sscratch = __trap_stack_base + (hartid + 1) * TRAP_STACK_SIZE
        "la   t0, __trap_stack_base\n"
        "li   t1, %1\n"
        "addi t2, tp, 1\n"
        "mul  t2, t2, t1\n"
        "add  t0, t0, t2\n"
        "csrw sscratch, t0\n"

        "j secondary_cpu_main\n"
        :
        : "i"(CPU_STACK_SIZE), "i"(TRAP_STACK_SIZE)
        : "t0", "t1", "t2"
    );
}

void start_secondary_cpus(void){

    for (int i = 1; i < NCPU; i++) {
        VERBOSE_PRINTF("Start %d/%d \n",i, NCPU);
        sbi_hart_start(i, (unsigned long) secondary_entry, 0);
    }    
}


__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void)
{
    __asm__ __volatile__(
        "mv tp, a0\n"                // tp = hartid

        // sp = __stack_base + (hartid + 1) * CPU_STACK_SIZE
        "la   t0, __stack_base\n"
        "li   t1, %0\n"
        "addi t2, tp, 1\n"
        "mul  t2, t2, t1\n"
        "add  sp, t0, t2\n"

        // sscratch = __trap_stack_base + (hartid + 1) * TRAP_STACK_SIZE
        "la   t0, __trap_stack_base\n"
        "li   t1, %1\n"
        "addi t2, tp, 1\n"
        "mul  t2, t2, t1\n"
        "add  t0, t0, t2\n"
        "csrw sscratch, t0\n"

        "j kmain\n"
        :
        : "i"(CPU_STACK_SIZE), "i"(TRAP_STACK_SIZE)
        : "t0", "t1", "t2"
    );
}