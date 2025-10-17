#include "inc/types.h"
#include "idt.h"
#include "trap.h"

static void create_idt_gate(uint8_t n, uint32_t handler) {
    idt[n].handler_low = handler & 0xFFFF;
    idt[n].handler_high = (handler >> 16) & 0xFFFF;
    idt[n].segsel = SEGSEL_KERNEL_CS;
    idt[n].zero = 0;
    idt[n].config = (1 << 7) | (0 << 5) | (0 << 3) | IDT_INTERRUPT_GATE;
}

extern void idt_load_and_set(uint32_t);

void idt_init() {
    idt_ptr_t idt_ptr;
    idt_ptr.limit = sizeof(idt_gate_t) * IDT_NUM_ENTRIES - 1;
    idt_ptr.base = (uint32_t)&idt;

    // por ahora, solo un timer / prueba
    create_idt_gate(0x20, (uint32_t)&trap_entry);

    idt_load_and_set((uint32_t)&idt_ptr);
}