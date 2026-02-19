/*
En x86, las interrupciones de hardware (IRQs) provienen del PIC (Programmable Interrupt Controller),
que maneja 15 líneas IRQ físicas (IRQ0–IRQ15).
*/

#include "idt.h"
#include "trap.h"
#include "interrupt.h"

#include "arch_inc/x86.h"


idt_gate_t idt[IDT_NUM_ENTRIES];
idt_ptr_t idt_ptr;

extern void idt_load_and_set(uint32_t idt_ptr);

void create_idt_gate(uint8_t n, uint32_t handler) {
    idt[n].handler_low  = handler & 0xFFFF;
    idt[n].handler_high = (handler >> 16) & 0xFFFF;
    idt[n].segsel = SEGSEL_KERNEL_CS;
    idt[n].zero = 0;
    idt[n].config = (1 << 7) | (0 << 5) | (0 << 3) | IDT_INTERRUPT_GATE;
}

void create_user_idt_gate(uint8_t n, uint32_t handler) {
    idt[n].handler_low  = handler & 0xFFFF;
    idt[n].handler_high = (handler >> 16) & 0xFFFF;
    idt[n].segsel       = SEGSEL_KERNEL_CS;
    idt[n].zero         = 0;

    idt[n].config =
        (1 << 7) |        // Present
        (3 << 5) |        // DPL = 3 (user callable)
        (0 << 3) |        // Reserved
        IDT_INTERRUPT_GATE;
}


// void create_user_idt_gate_with_irq(uint8_t n, uint32_t handler) {
//     idt[n].handler_low  = handler & 0xFFFF;
//     idt[n].handler_high = (handler >> 16) & 0xFFFF;
//     idt[n].segsel       = SEGSEL_KERNEL_CS;
//     idt[n].zero         = 0;

//     idt[n].config =
//         (1 << 7) |        // Present
//         (3 << 5) |        // DPL = 3
//         (0 << 3) |
//         IDT_TRAP_GATE;    // <-- trap, not interrupt
// }
void pic_acknowledge(int irq)
{
    if (irq >= 8)
        outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
//TODO: check
// void pic_acknowledge()
// {
//     outb(PIC1_PORT_A, PIC_EOI);
//     outb(PIC2_PORT_A, PIC_EOI);
// }
// void pic_acknowledge(int irq)
// {
//     if (irq >= 8)
//         outb(0xA0, 0x20);
//     outb(0x20, 0x20);
// }

#define NBASE_TRAPS 33
#define TOTAL_TRAPS NBASE_TRAPS+1



void idt_init(void) {
    idt_ptr.limit = sizeof(idt_gate_t) * IDT_NUM_ENTRIES - 1;
    idt_ptr.base  = (uint32_t)&idt;

    create_idt_gate(1, (uint32_t)isr1);
    create_idt_gate(2, (uint32_t)isr2);
    create_idt_gate(3, (uint32_t)isr3);
    create_idt_gate(4, (uint32_t)isr4);
    create_idt_gate(5, (uint32_t)isr5);
    create_idt_gate(6, (uint32_t)isr6);
    create_idt_gate(7, (uint32_t)isr7);
    create_idt_gate(8, (uint32_t)isr8);
    create_idt_gate(10, (uint32_t)isr10);
    create_idt_gate(11, (uint32_t)isr11);
    create_idt_gate(12, (uint32_t)isr12);
    create_idt_gate(13, (uint32_t)isr13);
    create_idt_gate(14, (uint32_t)isr14);
    create_idt_gate(16, (uint32_t)isr16);
    create_idt_gate(17, (uint32_t)isr17);
    create_idt_gate(18, (uint32_t)isr18);
    create_idt_gate(19, (uint32_t)isr19);

    create_idt_gate(T_IRQ0 + IRQ_TIMER, (uint32_t)isr32); // timer
    create_idt_gate(T_IRQ0 + IRQ_KBD, (uint32_t)isr33); // keyboard
    create_idt_gate(T_IRQ0 + IRQ_COM1, (uint32_t)isr36); // uart
    create_idt_gate(T_IRQ0 + IRQ_IDE, (uint32_t)isr46); // index: 14+32

    // mock proc
    create_user_idt_gate(0x80, (uint32_t)syscall_handler);

    lidt(idt, sizeof(idt));
}
