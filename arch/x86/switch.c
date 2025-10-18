#include "arch/switch.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"

// Cambia de contexto al proceso 'next'
// __attribute__((naked))
// void switch_context(struct Proc* next) {
//     __asm__ __volatile__ (
//         "pusha\n"                 // guarda todos los regs
//         "movl %esp, (%eax)\n"     // eax = &current->tf, guarda esp actual
//         "movl 4(%esp), %eax\n"    // argumento: next
//         "movl (%eax), %esp\n"     // carga esp de next->tf
//         "popa\n"
//         "ret\n"
//     );
// }

#include "arch_inc/trapframe.h"
#include "arch/proc.h"

__attribute__((naked))
void switch_context(TrapFrame *next_tf, struct Proc* next) {
    __asm__ __volatile__ (
        "movl 4(%esp), %eax\n"   /* eax = next_tf (argumento) */
        "movl %eax, %esp\n"      /* set stack pointer to start of TrapFrame */

        /* restore segment registers and general registers in the same order
           your irq stub would pop them before doing iret */
        "popl %gs\n"
        "popl %fs\n"
        "popl %es\n"
        "popl %ds\n"

        "popa\n"                 /* restores edi, esi, ebp, (esp skipped), ebx, edx, ecx, eax */

        /* skip int_no and err_code on the stack so that iret finds eip/cs/eflags */
        "addl $8, %esp\n"

        "iret\n"
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
    printf("[INIT TRAPFRAME] Dir memoria trapframe: %p\n", &tf);
    printf("[INIT TRAPFRAME] Dir memoria trapframe int_no: %p\n", &tf->int_no);

    /* segments (kernel) */
    tf->gs = 0;
    tf->fs = 0;
    tf->es = 0x10;  // si usás segmentos
    tf->ds = 0x10;

    /* registers (pusha order fields) */
    tf->edi = 0;
    tf->esi = 0;
    tf->ebp = 0;
    tf->esp_original = (uint32_t)(&proc->stack[SIZE_KERN_STACK]); // spare
    tf->ebx = 0;
    tf->edx = 0;
    tf->ecx = 0;
    tf->eax = 0;

    /* interrupt meta */
    printf("[INIT TRAPFRAME] Antes de setear tf->int_no %d\n", tf->int_no);
    tf->int_no = 0;
    printf("[INIT TRAPFRAME] Despues de setear tf->int_no %d\n", tf->int_no);
    tf->err_code = 0;

    /* CPU pushed fields (what iret will pop) */
    tf->eip = entry_point;
    tf->cs = 0x08;
    tf->eflags = 0x202;    /* IF = 1 */
    tf->useresp = (uint32_t)(&proc->stack[SIZE_KERN_STACK]); /* kernel stack top */
    tf->ss = 0x10;

    /* proc->tf is already the struct at that address so no extra copy needed */
}

/*
 * Actualiza el trapframe de un proceso con el contexto actual (por ejemplo, desde un trap).
 */
void update_trapframe(struct Proc *proc, TrapFrame *tf) {
    proc->tf = *tf;
}