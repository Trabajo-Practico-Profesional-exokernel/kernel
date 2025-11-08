#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "inc/types.h"
#include "inc/common.h"
#include "../../idt.h"
#include "../../gdt.h"
#include "../../interrupt.h"
#include "drivers/io/serial_handler.h"

void init_arch(void){
    disable_interrupts();
    serial_init();
    gdt_init();
    
    mem_init();
    //pde_init();
    vmmngr_initialize();
    idt_init();
    enable_interrupts();
}