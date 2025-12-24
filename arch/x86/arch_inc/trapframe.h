/*

A trap frame is a data structure used by the operating system to save
the state of a processor when an exception occurs, such as during a system call or an interrupt.
It contains information like the values of processor registers and the cause of the exception,
allowing the OS to manage the transition between user mode and kernel mode effectively.

Some typical fields in x86:

| Field Name | Description                                                                 |
|------------|-----------------------------------------------------------------------------|
| eflags     | Stores the flags register, indicating the state of the CPU.                 |
| cs         | Contains the code segment selector for the executing code.                  |
| eip        | Holds the instruction pointer, indicating where to resume execution.        |
| ss         | Contains the stack segment selector.                                        |
| esp        | Points to the current stack pointer.                                        |
| trapno     | Indicates the type of trap that occurred.                                   |
| error code | An optional field that may contain an error code for specific exceptions.   |


En tu FullTrapFrame ya tenés un campo oesp (el que empuja pusha), 
que representa el ESP del proceso en el momento del trap, justo antes de hacer pusha.

Pero también existe el stack pointer real que se usará al volver:
- Si estás en kernel mode todo el tiempo, ese valor es el mismo que esp_original.
- Si hay cambio de privilegio, entonces el useresp y ss del final de tu struct 
    son los verdaderos que restaurará iret.

*/

#ifndef TRAPFRAME_H
#define TRAPFRAME_H

#include "inc/types.h"

// For context switch between processes  
typedef struct PushTrapFrame {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t oesp; // esp antes del cambio. esta para que ande pusha/popa
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    // uint32_t eip;   // return address (instruction pointer)
    // uint32_t esp;   // stack pointer
} __attribute__((packed)) PushTrapFrame;


// For traps and interruptions
typedef struct TrapFrame {
    /* Segment registers pushed by your stub (push ds; push es; push fs; push gs) */
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    /* Registers as left by pusha: edi, esi, ebp, oesp, ebx, edx, ecx, eax */
    // uint32_t edi;
    // uint32_t esi;
    // uint32_t ebp;
    // uint32_t oesp; //ESP del proceso en el momento del trap
    // uint32_t ebx;
    // uint32_t edx;
    // uint32_t ecx;
    // uint32_t eax;
    PushTrapFrame regs;

    /* pushed by the stub just before calling handle_trap */
    uint32_t int_no;
    uint32_t err_code;

    /* pushed by CPU on interrupt entry (lower on stack) */
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    /* optional if ring change: useresp and ss (can be 0 for kernel-only) */
    uint32_t esp; //useresp
    uint32_t ss;
} __attribute__((packed)) TrapFrame;

typedef TrapFrame FullTrapFrame;



#endif


// TP SISOP
// struct PushRegs {
// 	/* registers as pushed by pusha */
// 	uint32_t reg_edi;
// 	uint32_t reg_esi;
// 	uint32_t reg_ebp;
// 	uint32_t reg_oesp; /* Useless */
// 	uint32_t reg_ebx;
// 	uint32_t reg_edx;
// 	uint32_t reg_ecx;
// 	uint32_t reg_eax;
// } __attribute__((packed));




// TODO: CAMBIAR PARA HACERLO CON x86
// Macros for syscalls For syscalls param and return handling
#define SYSCALL_ARG0(tf) tf->regs.ebx
#define SYSCALL_ARG1(tf) tf->regs.ecx
#define SYSCALL_ARG2(tf) tf->regs.edx
#define SYSCALL_SYSNO(tf) tf->regs.eax

#define SET_SYSCALL_RET0(tf, vl) tf->regs.eax=vl;
#define SET_SYSCALL_RET1(tf, vl) tf->regs.ebx=vl;


// Macro to define how to change stack base/top
#define SWITCH_TO_STACK(stack_base, stack_top)               \
        __asm__ volatile (                                   \
            "movl %0, %%esp\n"                               \
            "movl %1, %%ebp\n"                               \
            :                                                \
            : "r"(stack_top), "r"(stack_base)                \
            : "memory");                                     



