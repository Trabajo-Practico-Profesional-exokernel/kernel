#ifndef GDT_H
#define GDT_H

#include "tss.h"

#define SEGSEL_KERNEL_CS 0x08
#define SEGSEL_KERNEL_DS 0x10

#define PL0 0x0
#define PL3 0x3

#define SEG16(type, base, lim, dpl) (struct Segdesc)			\
{ (lim) & 0xffff, (base) & 0xffff, ((base) >> 16) & 0xff,		\
    type, 1, dpl, 1, (unsigned) (lim) >> 16, 0, 0, 1, 0,		\
    (unsigned) (base) >> 24 }

void gdt_init();

#endif /* GDT_H */

