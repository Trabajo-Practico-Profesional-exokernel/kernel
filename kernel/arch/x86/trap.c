#include "idt.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"
#include "io.h"
#include "constants.h"
#include "arch/logging.h"
#include "trap.h"
#include "stdio.h"
#include "console/debug.h"

extern void isr32(void);
void clock_yield(FullTrapFrame *tf, uintptr_t proc_pc);
#include "arch/trap_handling.h"


void init_trap(void) {
    //debug_printf("[TRAP] Initializing IDT and PIC...\n");

    //idt_init();
    //pic_remap();

    __asm__ __volatile__("sti"); // CHECK: if doesn't need to be here, only in switch_context
    debug_printf("[TRAP] Interrupts enabled\n");
}


unsigned long get_cr2_value(void) {
    unsigned long value;
    // The "mov %%cr2, %0" instruction moves the CR2 register value 
    // into the output operand %0, which is the 'value' variable.
    asm volatile("mov %%cr2, %0" : "=r" (value));
    return value;
}

/*
==================================================
  HANDLER GENERAL
==================================================
*/

//No toca sti (eso se hace en el stub después del iret)
void handle_trap(FullTrapFrame *tf) {
    switch (tf->int_no) {
        case (T_IRQ0 + IRQ_TIMER): // Timer IRQ
            // End of interrupt (solo master, IRQ0)
            outb(0x20, 0x20);
            clock_yield(tf, tf->eip);
            break;
        case (T_IRQ0 + IRQ_KBD):
            printf("KA");
            keyboard_handle_interrupt();
            break;

        case (T_IRQ0 + IRQ_COM1):
            keyboard_handle_interrupt();
            pic_acknowledge(4); // Notificar al PIC (IRQ 4)
            break;

        case (T_IRQ0 + IRQ_IDE):
            pic_acknowledge(4); // Notificar al PIC (IRQ 4)
            break;

        case T_PGFLT: 
            unsigned long addr_fault = get_cr2_value();
            printTrapFull(tf);
            
            // EIP is not actually where it happened! Allegedly its on the stack?
            PANIC("\n[TRAP] Pagefault at %x fault address: 0x%x \n", tf->eip, addr_fault);
            break;

        case T_SYSCALL:
            uintptr_t user_pc = handle_syscall(tf, tf->eip);
            // PANIC("\n[TRAP] Dont know how to go back wiuth new pc %x\n", user_pc);
            break;

        default:
            printTrapFull(tf);
            PANIC("\n[TRAP] Unhandled trap %u \n", tf->int_no);
            break;
    }
}
