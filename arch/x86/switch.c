#include "arch/switch.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"

// Cambia de contexto al proceso 'next'
__attribute__((naked))
void switch_context(struct Proc* next) {
    __asm__ __volatile__ (
        "pusha\n"                 // guarda todos los regs
        "movl %esp, (%eax)\n"     // eax = &current->tf, guarda esp actual
        "movl 4(%esp), %eax\n"    // argumento: next
        "movl (%eax), %esp\n"     // carga esp de next->tf
        "popa\n"
        "ret\n"
    );
}

/*
 * En x86, 'sleep' es igual de simple: hace busy-wait.
 */
void sleep(int delay) {
    for (int i = 0; i < delay; i++)
        __asm__ __volatile__("nop");
}

/*
 * Inicializa el TrapFrame de un nuevo proceso para que, al hacer 'switch_context',
 * comience en la instrucción 'init_ins' (función de entrada del proceso).
 */
void init_trapframe(struct Proc *proc, uint32_t entry_point) {
    TrapFrame *tf = &proc->tf;

    tf->edi = 0;
    tf->esi = 0;
    tf->ebp = 0;
    tf->esp_dummy = 0;
    tf->ebx = 0;
    tf->edx = 0;
    tf->ecx = 0;
    tf->eax = 0;

    tf->int_no = 0;
    tf->err_code = 0;

    // Dirección de instrucción inicial
    tf->eip = entry_point;

    // Segmento de código y datos kernel (normalmente GDT 0x08, 0x10)
    tf->cs = 0x08;      // Código kernel
    tf->eflags = 0x202; // IF = 1 habilita interrupciones
    tf->useresp = (uint32_t)(&proc->stack[SIZE_KERN_STACK]);
    tf->ss = 0x10;      // Segmento de datos kernel

    // Esto es importante: el stack pointer inicial
    // (cuando hagas switch_context, el CPU saltará aquí)
    proc->tf = *tf;
}

/*
 * Actualiza el trapframe de un proceso con el contexto actual (por ejemplo, desde un trap).
 */
void update_trapframe(struct Proc *proc, TrapFrame *tf) {
    proc->tf = *tf;
}