#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "inc/types.h"
#include "inc/common.h"
#include "../../cpu.h"
#include "../../idt.h"
#include "../../gdt.h"
#include "../../interrupt.h"
#include "drivers/io/serial_handler.h"

static void cpu_init(void){
    tss_t tss = { .prev_tss = NULL, .esp0 = 0, .ss0 = 0 };
    cpu = (CpuInfo){ .cpu_id = 0, .cpu_status = CPU_STARTED, .cpu_ts = tss };
}


void init_arch(void){
    disable_interrupts();
    serial_init();
    gdt_init();
    
    mem_init();
    //pde_init();
    vmmngr_initialize();
    idt_init();
    cpu_init
}