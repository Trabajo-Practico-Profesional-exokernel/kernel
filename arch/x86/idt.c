/*
En x86, las interrupciones de hardware (IRQs) provienen del PIC (Programmable Interrupt Controller),
que maneja 15 líneas IRQ físicas (IRQ0–IRQ15).
*/

#include "idt.h"
#include "trap.h"
#include "interrupt.h"
#include "drivers/io.h"


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

void (*isr_table[TOTAL_TRAPS])(void) = {
    0,      // 0 no usado o divide-by-zero si vos querés
    isr1,
    isr2,
    isr3,
    isr4,
    isr5,
    isr6,
    isr7,
    isr8,
    isr9,
    isr10,
    isr11,
    isr12,
    isr13,
    isr14,
    isr15,
    isr16,
    isr17,
    isr18,
    isr19,
    isr20,
    isr21,
    isr22,
    isr23,
    isr24,
    isr25,
    isr26,
    isr27,
    isr28,
    isr29,
    isr30,
    isr31,
    isr32,
    isr33,
};


void idt_init(void) {
    idt_ptr.limit = sizeof(idt_gate_t) * IDT_NUM_ENTRIES - 1;
    idt_ptr.base  = (uint32_t)&idt;

    for (int i = 1; i < 32; i++) {
        create_idt_gate(i, (uint32_t)isr_table[i]);
    }

    create_idt_gate(32, (uint32_t)isr32); // timer
    create_idt_gate(33, (uint32_t)isr33); // keyboard

    // mock proc
    create_user_idt_gate(0x80, (uint32_t)syscall_handler);

    idt_load_and_set((uint32_t)&idt_ptr);
}