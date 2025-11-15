#ifndef GDT_H
#define GDT_H

#include "tss.h"

#define SEGSEL_KERNEL_CS 0x08
#define SEGSEL_KERNEL_DS 0x10

#define GD_KT  0x08 // kernel code/text 
#define GD_KD  0x10 // kernel data 
#define GD_UT  0x18 // user code/text 
#define GD_UD  0x20 // user data 
#define GD_TSS 0x28 // task state 

#define SEG_KT  (GD_KT >> 3)  // segment kernel code
#define SEG_KD  (GD_KD >> 3)  // kernel data+stack
#define SEG_UT  (GD_UT >> 3)  // kernel code/text 
#define SEG_UD  (GD_UD >> 3)  // kernel data+stack
#define SEG_TSS (GD_TSS >> 3) // current process task state 

#define PL0 0x0
#define PL3 0x3

/*
    segment descriptor is 8 byte (64 bit) long:
    
    31                   16 15                    0
    /---------------------------------------------/
    |      base (0:15)     |     limit (0:15)     |
    /---------------------------------------------/

    63    56 55   52 51   48 47      40 39       32
    /---------------------------------------------/
    | base  | flags | limit |  access  |   base   |
    |(24:31)|       |(16:19)|   byte   | (16:23)  |
    /---------------------------------------------/

    base: segment linear address

    limit: the maximum addressable unit

                                 / ~~~~ type ~~~~~ /
                   7   6   5   4   3    2    1   0
    access byte: | P |  DPL  | S | E | DC | RW | A |
        * P   [1bit]:   present bit 
        * DPL [2bit]:   descriptor privilege level
        * S   [1bit]:   (0) system segment; (1) code/data segment 

      (next is only for code/data segments, S=1)
        * E   [1bit]:   exec bit; (0) data segment; (1) code segment
        * DC  [1bit]:   direction/conforming bit;
                            data -> (0) grows up; 
                                    (1) grows down;
                            code -> (0) can be executed only from the ring set in DPL;
                                    (1) can be executed from an equal or lower privilege level (0 highest; 3 lowest) 
        * R/W [1bit]:   read/write bit: readable for code; writeable for data (read is always allowed for data);                          
        * A   [1bit]:   accesed bit: CPU will set it when the segment is accessed unless set to 1 in advance
    

    flags: 
          3    2   1       0
        | G | DB | L | reserved | 
          |   |    *-> long-mode code: if 1 defines 64bit code segment (DB=0).
          |   *-> size: if 0 defines 16bit protected mode segment, else 32bit protected mode segment.
          *-> granularity: if 0 limit is in bytes, else limit is in 4KB blocks (pages).


*/

/*  Segment descriptor as described above */
struct Segdesc {
    unsigned int limit_low              : 16;
    unsigned int base_low               : 24;
    unsigned int type                   :  4; // STS_ constants
    unsigned int s                      :  1; // 0 = system; 1 = code/data 
    unsigned int dpl                    :  2; // privilege level
    unsigned int p                      :  1; // present
    unsigned int limit_high             :  4;
    unsigned int rsv                    :  1; // reserved 
    unsigned int l                      :  1; // long-mode code
    unsigned int db                     :  1; // size 
    unsigned int g                      :  1; // granularity 
    unsigned int base_high              :  8; 
};  

#define SEG_NULL (struct segdesc) {0}

#define SEG(type, base, lim, dpl) (struct Segdesc)		    	\
    { (lim) & 0xffff,                                           \
      (base) & 0xffffff,                                        \
      type,                                                     \
      1,                                                        \
      dpl,                                                      \
      1,                                                        \
      (unsigned) (lim) >> 16,                                   \
      0,                                                        \
      0,                                                        \
      1,                                                        \
      1,		                                                \
      (unsigned) (base) >> 24 }                                   

#define SEG16(type, base, lim, dpl) (struct Segdesc)			\
    { (lim) & 0xffff,                                           \
      (base) & 0xffffff,                                        \
      type,                                                     \
      1,                                                        \
      dpl,                                                      \
      1,                                                        \
      (unsigned) (lim) >> 16,                                   \
      0,                                                        \
      0,                                                        \
      1,                                                        \
      0,		                                                \
      (unsigned) (base) >> 24 }                                   


// System segment type bits 
// (see: Intel 64 and IA-32 Architectures Software Developer’s Manual; 
//       Section 3.5 SYSTEM DESCRIPTOR TYPES)
#define STS_T16A    0x1     // Available 16-bit TSS
#define STS_LDT     0x2     // Local Descriptor Table
#define STS_T16B    0x3     // Busy 16-bit TSS
#define STS_CG16    0x4     // 16-bit Call Gate
#define STS_TG      0x5     // Task Gate / Coum Transmitions
#define STS_IG16    0x6     // 16-bit Interrupt Gate
#define STS_TG16    0x7     // 16-bit Trap Gate
#define STS_T32A    0x9     // Available 32-bit TSS
#define STS_T32B    0xB     // Busy 32-bit TSS
#define STS_CG32    0xC     // 32-bit Call Gate
#define STS_IG32    0xE     // 32-bit Interrupt Gate
#define STS_TG32    0xF     // 32-bit Trap Gate

// Application segment type bits 
// (see: Intel 64 and IA-32 Architectures Software Developer’s Manual; 
//       Section 3.4.5.1 Code- and Data-Segment Descriptor Types)
#define STA_X       0x8     // Executable segment
#define STA_E       0x4     // Expand down (non-executable segments)
#define STA_C       0x4     // Conforming code segment (executable only)
#define STA_W       0x2     // Writeable (non-executable segments)
#define STA_R       0x2     // Readable (executable segments)
#define STA_A       0x1     // Accessed




void gdt_init();

#endif /* GDT_H */

