#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "arch/mem.h"
#include "inc/types.h"
#include "inc/common.h"
#include "../../cpu.h"
#include "../../idt.h"
#include "../../gdt.h"
#include "../../interrupt.h"
#include "drivers/io/serial_handler.h"

struct CpuInfo cpus[NCPU];
#define GD_KD  0x10

extern char __trap_stack_top[];

static void cpu_init(void){
    struct TaskState ts = { .prev_tss = 0, .esp0 = __trap_stack_top, .ss0 = GD_KD };
	cpu->cpu_id = 0;
	cpu->cpu_status = CPU_STARTED;

    // TODO: hacerlo mas lindo (e investigar)
    ts.cs=0x0b;
	ts.ss = 0x13;
	ts.es = 0x13;
	ts.ds = 0x13;
	ts.fs = 0x13;
	ts.gs = 0x13;

    cpu->cpu_ts = ts;
}


void init_arch(void){
    disable_interrupts();
    serial_init();
    gdt_init();
    
    //mem_init();
    //pde_init();
    // vmmngr_initialize();
    idt_init();
    cpu_init();
}
