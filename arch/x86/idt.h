#ifndef IDT_H
#define IDT_H

#include "inc/types.h"

#define IDT_NUM_ENTRIES 256
#define SEGSEL_KERNEL_CS 0x08
#define IDT_INTERRUPT_GATE 0x0E

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

#endif