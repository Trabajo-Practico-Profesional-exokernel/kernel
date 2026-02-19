#include "gdt.h"
#include "arch_inc/mem_constants.h"

#define MPENTRY_PADDR 0x7000
#define MPBOOTPHYS(s) ((s) - entryother_start + MPENTRY_PADDR)

.org 0x7000
.code16
.globl entryother_start
entryother_start:
    cli

    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
 
    lgdt MPBOOTPHYS(gdtdesc)
    movl %cr0, %eax
    orl $(CR0_PE), %eax 
    movl %eax, %cr0
    
    ljmp $(GD_KT), $(MPBOOTPHYS(start32))

.code32
start32:
    movw $(GD_KD), %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    movw $0, %ax
    movw %ax, %fs
    movw %ax, %gs

    movl %cr4, %eax
    orl $(CR4_PSE), %eax
    movl %eax, %cr4

    movl kernel_pde, %eax
    movl %eax, %cr3

    movl %cr0, %eax
    orl $(CR0_PE|CR0_PG|CR0_WP), %eax 
    movl %eax, %cr0

    movl cpu_stack, %esp
    movl $0, %ebp

    movl $init_arch_others, %eax
    call *%eax

    movw    $0x8a00, %ax
    movw    %ax, %dx
    outw    %ax, %dx
    movw    $0x8ae0, %ax
    outw    %ax, %dx

spin:
    jmp spin


# Bootstrap GDT
.p2align 2                  # force 4 byte alignment
_gdt:
    SEG_NULL                            # null seg
    SEG(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
    SEG(STA_W, 0x0, 0xffffffff)     # data seg

gdtdesc:
    .word   gdtdesc - _gdt - 1         # sizeof(gdt) - 1
    .long   MPBOOTPHYS(_gdt)     # address gdt


.globl entryother_end
entryother_end:

