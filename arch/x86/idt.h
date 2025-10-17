#include "inc/types.h"

#define IDT_NUM_ENTRIES 256
#define SEGSEL_KERNEL_CS 0x08  // de tu GDT
#define IDT_INTERRUPT_GATE 0x0E

struct idt_gate {
    uint16_t handler_low;
    uint16_t segsel;
    uint8_t zero;
    uint8_t config;
    uint16_t handler_high;
} __attribute__((packed));
typedef struct idt_gate idt_gate_t;

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct idt_ptr idt_ptr_t;

idt_gate_t idt[IDT_NUM_ENTRIES];

static void create_idt_gate(uint8_t n, uint32_t handler);

void idt_init();