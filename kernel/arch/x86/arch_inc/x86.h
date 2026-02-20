#ifndef X86_H
#define X86_H

#include "types.h"

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
lgdt(void *p, uint32_t size)
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

static inline void lseg(void) {
    asm volatile (
        "movw $0x10, %%ax\n\t"   // 0x10 = kernel data selector
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%ss\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        "ljmp $0x08, $1f\n\t"    // 0x08 = kernel code selector
        "1:\n\t"
        :
        :
        : "ax"
    );
}

static inline uint32_t
read_eflags(void)
{
	uint32_t eflags;
	asm volatile("pushfl; popl %0" : "=r" (eflags));
	return eflags;
}

static inline uint32_t
xchg(volatile uint32_t *addr, uint32_t newval)
{
  uint32_t result;

  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
}

static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t lo, hi;
    __asm__ volatile (
        "rdmsr"
        : "=a"(lo), "=d"(hi)
        : "c"(msr)
    );
    return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr, uint64_t val)
{
    uint32_t lo = (uint32_t)val;
    uint32_t hi = (uint32_t)(val >> 32);
    __asm__ volatile (
        "wrmsr"
        :
        : "c"(msr), "a"(lo), "d"(hi)
    );
}

// -------------------------------
// E/S de puertos (in/out)
// -------------------------------

static inline void outb(uint16_t port, uint8_t value) {
     __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ __volatile__("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
	uint16_t ret;
	__asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void outl(uint16_t port, uint32_t val)
{
    __asm__ volatile (
        "outl %0, %1"
        :
        : "a"(val), "Nd"(port)
        : "memory"
    );
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ __volatile__("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void
insl(uint32_t port, void *addr, uint32_t cnt)
{
	asm volatile("cld\n\trepne\n\tinsl"
		     : "=D" (addr), "=c" (cnt)
		     : "d" (port), "0" (addr), "1" (cnt)
		     : "memory", "cc");
}

static inline void
outsl(uint16_t port, const void *addr, uint32_t cnt)
{
	asm volatile("cld\n\trepne\n\toutsl"
		     : "=S" (addr), "=c" (cnt)
		     : "d" (port), "0" (addr), "1" (cnt)
		     : "cc");
}

static inline void 
insw(uint16_t port, void *addr, uint32_t cnt)
{
    asm volatile (
        "cld\n\t"
        "rep insw"
        :
        : "d"(port), "D"(addr), "c"(cnt)
        : "memory"
    );
}

static inline void
outsw(uint16_t port, const void *addr, uint32_t cnt)
{
    asm volatile (
        "cld\n\t"
        "rep outsw"
        :
        : "d"(port), "S"(addr), "c"(cnt)
        : "memory"
    );
}

static inline void 
insb(uint16_t port, void *addr, uint32_t cnt)
{
    asm volatile (
        "cld\n\t"
        "rep insb"
        :
        : "d"(port), "D"(addr), "c"(cnt)
        : "memory"
    );
}

static inline void
outsb(uint16_t port, const void *addr, uint32_t cnt)
{
    asm volatile (
        "cld\n\t"
        "rep outsb"
        :
        : "d"(port), "S"(addr), "c"(cnt)
        : "memory"
    );
}

// -------------------------------
// IDT (Interrupt Descriptor Table)
// -------------------------------

static inline uint32_t rcr3(void)
{
    uint32_t val;
    __asm__ volatile ("mov %%cr3, %0"
                      : "=r"(val)
                      :
                      : );
    return val;
}


// Carga la IDT en el registro IDTR
static inline void
lidt(void *p, uint32_t size)
{
  volatile uint16_t pd[3];

  pd[0] = size-1;
  pd[1] = (uint32_t)p;
  pd[2] = (uint32_t)p >> 16;

  asm volatile("lidt (%0)" : : "r" (pd));
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
