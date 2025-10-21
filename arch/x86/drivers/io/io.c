#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "inc/types.h"
#include "inc/common.h"
#include "../../idt.h"
#include "../../gdt.h"
#include "../../interrupt.h"

void init_arch(void){
    disable_interrupts();
    gdt_init();
    //pic_init();
    idt_init();
    enable_interrupts();
}