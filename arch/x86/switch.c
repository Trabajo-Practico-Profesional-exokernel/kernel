#include "arch/switch.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"
#include "inc/common.h"

__attribute__((naked))
void switch_context(struct Proc *next) {
    __asm__ __volatile__ (
        // a0 (RISC-V) ≈ first arg in x86 -> [esp + 4]
        "mov 4(%esp), %eax\n"         // eax = next (struct Proc*)
        "mov (%eax), %eax\n"          // eax = next->tf (tf es el primer campo de proc)

        // Restaurar registros del TrapFrame
        "mov 0x00(%eax), %edi\n"      // edi
        "mov 0x04(%eax), %esi\n"      // esi
        "mov 0x08(%eax), %ebp\n"      // ebp
        "mov 0x10(%eax), %ebx\n"      // ebx (saltamos oesp)
        "mov 0x14(%eax), %edx\n"      // edx
        "mov 0x18(%eax), %ecx\n"      // ecx
        "mov 0x1C(%eax), %eax\n"      // eax

        // Cambiar el stack pointer al del proceso nuevo
        "mov 0x24(%eax), %esp\n"      // esp = tf->esp

        // Saltar a la instrucción de inicio del proceso
        "jmp *0x20(%eax)\n"           // eip = tf->eip
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

    tf->edi = 0;
    tf->esi = 0;
    tf->ebp = 0;
    tf->oesp = 0; 
    tf->ebx = 0;
    tf->edx = 0;
    tf->ecx = 0;
    tf->eax = 0;

    // Configura punto de inicio (eip)
    tf->eip = entry_point;

    tf->esp = (uint32_t)&proc->stack[sizeof(proc->stack)];

    printf("[INIT TRAPFRAME] entry_point = 0x%x, esp = 0x%x\n",tf->eip, tf->esp);
}

/*
 * Actualiza el trapframe de un proceso con el contexto actual (por ejemplo, desde un trap).
 */
void update_trapframe(struct Proc *proc, FullTrapFrame *tf) {
    proc->tf.eax = tf->eax;
    proc->tf.ebx = tf->ebx;
    proc->tf.ecx = tf->ecx;
    proc->tf.edx = tf->edx;
    proc->tf.esi = tf->esi;
    proc->tf.edi = tf->edi;
    proc->tf.ebp = tf->ebp;
    proc->tf.eip = tf->eip;

    proc->tf.oesp = tf->oesp; // TODO: porlas, ver si sacar
    proc->tf.esp = tf->useresp ? tf->useresp : tf->oesp; // TODO: ver si sacar
}