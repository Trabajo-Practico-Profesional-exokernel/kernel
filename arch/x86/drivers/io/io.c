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

static void cpu_init(void){
    struct TaskState ts = { .prev_tss = 0, .esp0 = 0, .ss0 = 0 };
	cpu->cpu_id = 0;
	cpu->cpu_status = CPU_STARTED;
	cpu->cpu_ts = ts;
}


void init_arch(void){
    disable_interrupts();
    serial_init();
    gdt_init();
    
    mem_init();
    //pde_init();
    // vmmngr_initialize();
    idt_init();
    cpu_init();
}
