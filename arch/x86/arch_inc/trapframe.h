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

*/

#ifndef TRAPFRAME_H
#define TRAPFRAME_H

#include "inc/types.h"

typedef struct TrapFrame {
    // Pushed by pusha
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy; // esp value before pusha
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    // Pushed manually or by CPU on interrupt
    uint32_t int_no;
    uint32_t err_code;

    // Pushed automatically by CPU
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;

    // TODO: Resolver esto para que sea agnostico a arquitctura
    uint32_t sp;   // agregado para compatibilidad con RISC-V
} __attribute__((packed)) TrapFrame;

typedef TrapFrame FullTrapFrame;

#endif