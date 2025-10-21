/*
En x86, las interrupciones de hardware (IRQs) provienen del PIC (Programmable Interrupt Controller),
que maneja 15 líneas IRQ físicas (IRQ0–IRQ15).
*/


#include "idt.h"
#include "trap.h"

idt_gate_t idt[IDT_NUM_ENTRIES];

void create_idt_gate(uint8_t n, uint32_t handler) {
    idt[n].handler_low  = handler & 0xFFFF;
    idt[n].handler_high = (handler >> 16) & 0xFFFF;
    idt[n].segsel = SEGSEL_KERNEL_CS;
    idt[n].zero = 0;
    idt[n].config = (1 << 7) | (0 << 5) | (0 << 3) | IDT_INTERRUPT_GATE;
}

extern void idt_load_and_set(uint32_t idt_ptr);

extern void isr32(void); // timer handler ASM

void idt_init(void) {
    idt_ptr_t idt_ptr;
    idt_ptr.limit = sizeof(idt_gate_t) * IDT_NUM_ENTRIES - 1;
    idt_ptr.base  = (uint32_t)&idt;

    create_idt_gate(0x20, (uint32_t)isr32); // IRQ0 - timer

    idt_load_and_set((uint32_t)&idt_ptr);
}