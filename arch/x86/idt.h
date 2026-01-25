#ifndef IDT_H
#define IDT_H

#include "types.h"

#define IDT_NUM_ENTRIES 256
#define SEGSEL_KERNEL_CS 0x08
#define IDT_INTERRUPT_GATE 0x0E

// TODO: check
// ---For Keyboard interrupts (aenix)---
#define PIC1_PORT_A 0x20
#define PIC2_PORT_A 0xA0
#define PIC_EOI     0x20

#define PIC1_START      0x20
#define PIC2_START      0x28
#define PIC_NUM_IRQS    16

#define PIT_INT_IDX     PIC1_START
#define KBD_INT_IDX     PIC1_START + 1

#define COM1_INT_IDX    PIC1_START + 4
#define COM2_INT_IDX    PIC1_START + 3
// -------------------------------------

typedef struct {
    uint16_t handler_low;
    uint16_t segsel;
    uint8_t zero;
    uint8_t config;
    uint16_t handler_high;
} __attribute__((packed)) idt_gate_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

extern idt_gate_t idt[IDT_NUM_ENTRIES];

void idt_init(void);
void create_idt_gate(uint8_t n, uint32_t handler);
void create_idt_gate_and_load(uint8_t n, uint32_t handler);
void pic_acknowledge(int irq);

#endif