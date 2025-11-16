#ifndef INC_TRAP_FRAME
#define INC_TRAP_FRAME

#include "inc/types.h"

// riscv5 specific full registers the ones for trap handling?
struct FullTrap {
    uint32_t ra;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
} __attribute__((packed));

typedef struct FullTrap FullTrapFrame;

// Process trap frame arguably the ones that would be needed for context switch, it seems the 1000 line os does use some registers for kernel
struct TrapFrame {
    uint32_t ra;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
} __attribute__((packed));



// Macros for stack pointer/switching
#define SWITCH_TO_STACK(stack_top) \
    __asm__ volatile("mv sp, %0" :: "r"(stack_top) :);

#define SSCRATCH_STACK() \
    __asm__ volatile("csrw sscratch, sp" :: :);

#define SSCRATCH_NEW_STACK(stack_top) \
    __asm__ volatile("csrw sscratch, %0" :: "r"(stack_top) :);

// Macros for syscalls For syscalls param and return handling
#define SYSCALL_ARG0(tf) tf->a0
#define SYSCALL_ARG1(tf) tf->a1
#define SYSCALL_ARG2(tf) tf->a2
#define SYSCALL_SYSNO(tf) tf->a3

#define SET_SYSCALL_RET0(tf, vl) tf->a0=vl;

#endif /* !*/
