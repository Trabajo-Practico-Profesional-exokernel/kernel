/*
En x86, las interrupciones de hardware (IRQs) provienen del PIC (Programmable Interrupt Controller),
que maneja 15 líneas IRQ físicas (IRQ0–IRQ15).
*/

#include "idt.h"
#include "trap.h"
#include "interrupt.h"


idt_gate_t idt[IDT_NUM_ENTRIES];

void create_idt_gate(uint8_t n, uint32_t handler) {
    idt[n].handler_low  = handler & 0xFFFF;
    idt[n].handler_high = (handler >> 16) & 0xFFFF;
    idt[n].segsel = SEGSEL_KERNEL_CS;
    idt[n].zero = 0;
    idt[n].config = (1 << 7) | (0 << 5) | (0 << 3) | IDT_INTERRUPT_GATE;
}

extern void idt_load_and_set(uint32_t idt_ptr);

void (*isr_table[33])(void) = {
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
    isr32
};


void idt_init(void) {
    idt_ptr_t idt_ptr;
    idt_ptr.limit = sizeof(idt_gate_t) * IDT_NUM_ENTRIES - 1;
    idt_ptr.base  = (uint32_t)&idt;

    for (int i = 1; i <= 32; i++) {
        create_idt_gate(i, (uint32_t)isr_table[i]);
    }

    // mock proc
    create_idt_gate(0x80, (uint32_t)syscall_handler);

    idt_load_and_set((uint32_t)&idt_ptr);
}