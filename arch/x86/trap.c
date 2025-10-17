#include "arch_inc/trapframe.h"
#include "arch/trap.h"
#include "arch/proc.h"
#include "inc/common.h"
#include "io.h"        // outb / inb para PIC
#include "arch_inc/x86.h" // define lidt, sti, cli, struct IDTEntry, etc

// Declaración de scheduler (como en riscv)
void sched_yield(struct TrapFrame *tf);

/*
==================================================
  INIT_TRAP: configura IDT + PIC + habilita interrupciones
==================================================
*/

extern void trap_entry(void);  // definida en asm (ver más abajo)

#define IDT_ENTRIES 256

struct IDTEntry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct IDTPtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct IDTEntry idt[IDT_ENTRIES];
static struct IDTPtr idt_ptr;

static void set_idt_gate(int n, uint32_t handler) {
    idt[n].base_low  = handler & 0xFFFF;
    idt[n].sel       = 0x08; // Kernel code segment
    idt[n].always0   = 0;
    idt[n].flags     = 0x8E; // Present, ring 0, 32-bit interrupt gate
    idt[n].base_high = (handler >> 16) & 0xFFFF;
}

void init_trap(void) {
    printf("[TRAP] Initializing IDT and timer interrupt...\n");

    // Configura la entrada para el timer (IRQ0 -> int 32)
    extern void irq0_handler(void); // definida abajo
    set_idt_gate(32, (uint32_t)irq0_handler);

    // Configura la IDT general
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    __asm__ __volatile__("lidt (%0)" :: "r"(&idt_ptr));
    printf("[TRAP] IDT loaded at %p\n", &idt);

    // Reprograma el PIC
    outb(0x20, 0x11); // init master
    outb(0xA0, 0x11); // init slave
    outb(0x21, 0x20); // master offset = 0x20
    outb(0xA1, 0x28); // slave offset = 0x28
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    // Habilita interrupciones globales
    __asm__ __volatile__("sti");
    printf("[TRAP] Interrupts enabled\n");
}


/*
==================================================
  HANDLER GENERAL
==================================================
*/

void handle_trap(struct TrapFrame *tf) {
    switch (tf->int_no) {
        case 32: // IRQ0 - Timer interrupt
            outb(0x20, 0x20); // EOI al PIC master
            sched_yield(tf);  // Llama al scheduler
            break;

        default:
            printf("[TRAP] Unhandled interrupt: %d\n", tf->int_no);
            break;
    }
}