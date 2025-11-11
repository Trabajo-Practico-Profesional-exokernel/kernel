#ifndef X86_H
#define X86_H

#include "inc/types.h"
#include "../gdt.h"

// -------------------------------
// Control de interrupciones
// -------------------------------

static inline void cli(void) {
    __asm__ __volatile__("cli");
}

static inline void sti(void) {
    __asm__ __volatile__("sti");
}

// -------------------------------
// Instrucción hlt (detener CPU)
// -------------------------------

static inline void hlt(void) {
    __asm__ __volatile__("hlt");
}

static inline void
lgdt(struct Segdesc *p, uint32_t size)
{
  volatile uint16_t pd[3];

  pd[0] = size-1;
  pd[1] = (uint32_t)p;
  pd[2] = (uint32_t)p >> 16;

  asm volatile("lgdt (%0)" : : "r" (pd));
}

static inline void
ltr(uint16_t sel)
{
  asm volatile("ltr %0" : : "r" (sel));
}

// -------------------------------
// E/S de puertos (in/out)
// -------------------------------

// static inline void outb(uint16_t port, uint8_t value) {
//     __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
// }

// static inline uint8_t inb(uint16_t port) {
//     uint8_t ret;
//     __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
//     return ret;
// }

// static inline void outw(uint16_t port, uint16_t value) {
//     __asm__ __volatile__("outw %0, %1" : : "a"(value), "Nd"(port));
// }

// static inline uint16_t inw(uint16_t port) {
//     uint16_t ret;
//     __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
//     return ret;
// }

// -------------------------------
// IDT (Interrupt Descriptor Table)
// -------------------------------


// Carga la IDT en el registro IDTR
static inline void lidt(struct IDTPointer* idt_ptr) {
    __asm__ __volatile__("lidtl (%0)" : : "r"(idt_ptr));
}

// -------------------------------
// Macros útiles para crear entradas IDT
// -------------------------------

#define IDT_PRESENT 0x80
#define IDT_INT_GATE 0x0E
#define IDT_TRAP_GATE 0x0F
#define IDT_RING0 0x00
#define IDT_RING3 0x60

#define SET_IDT_ENTRY(num, handler)                      \
    do {                                                 \
        uint32_t base = (uint32_t)(handler);             \
        idt[num].offset_low = base & 0xFFFF;             \
        idt[num].selector = 0x08; /* kernel code seg */  \
        idt[num].zero = 0;                               \
        idt[num].type_attr = IDT_PRESENT | IDT_INT_GATE; \
        idt[num].offset_high = (base >> 16) & 0xFFFF;    \
    } while (0)

#endif // X86_H
