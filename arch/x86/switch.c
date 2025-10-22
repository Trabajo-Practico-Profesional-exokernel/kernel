#include "arch/switch.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"
#include "inc/common.h"


// cuando tengamos espacio de usuario, cambiar por iret que restaura eip y esp
__attribute__((naked))
void switch_context(struct Proc *next) {
    __asm__ __volatile__ (
        // vaciar registros FullTrapFrame
        // "pop gs\n"
        // "pop fs\n"
        // "pop es\n"
        // "pop ds\n"
        
        "mov 4(%esp), %esp\n"         // eax = next (struct Proc*

        // Restaurar registros del TrapFrame
        "pop %edi\n"      // edi
        "pop %esi\n"      // esi
        "pop %ebp\n"      // ebp
        "pop %ebx\n"      // ebx (saltamos oesp)
        "pop %ebx\n"      // ebx 
        "pop %edx\n"      // edx
        "pop %ecx\n"      // ecx
        "pop %eax\n"      // eax

        "sti\n"
        "ret\n"           // eip = tf->eip
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
 *  uint32_t entry_point is in proc->pc
 */
void init_trapframe(struct Proc *proc) {
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
    tf->eip = proc->pc;

    tf->esp = proc->kernel_sp; 
    // For now? not good? lol at least it should not be 0 or so.. should be virtual addr

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