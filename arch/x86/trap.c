#include "idt.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"
#include "io.h"
#include "inc/common.h"

extern void isr32(void);
void sched_yield(FullTrapFrame *tf);

/*
==================================================
  PIC remap
==================================================
*/
static void pic_remap(void) {
    // ICW1
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    // ICW2 - vector offsets
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    // ICW3 - chaining master/slave
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    // ICW4
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    // Unmask (habilitar todas)
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void init_trap(void) {
    printf("[TRAP] Initializing IDT and PIC...\n");

    //idt_init();
    pic_remap();

    __asm__ __volatile__("sti"); // CHECK: if doesn't need to be here, only in switch_context
    printf("[TRAP] Interrupts enabled\n");
}

/*
==================================================
  HANDLER GENERAL
==================================================
*/

//No toca sti (eso se hace en el stub después del iret)
void handle_trap(FullTrapFrame *tf) {
    switch (tf->int_no) {
        case 32: // Timer IRQ
            // End of interrupt (solo master, IRQ0)
            outb(0x20, 0x20);
            sched_yield(tf);
            break;
        default:
            printf("[TRAP] Unhandled interrupt %d\n", tf->int_no);
            break;
    }
}
