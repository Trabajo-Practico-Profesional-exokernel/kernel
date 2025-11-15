#include "arch/switch.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"
#include "inc/common.h"

#define GD_KT  0x08 // kernel code/text 
#define GD_KD  0x10 // kernel data 
#define GD_UT  0x18 // user code/text 
#define GD_UD  0x20 // user data 
#define GD_TSS 0x28 // task state
#define FL_IF  0x00000200 // flag for interrupt (so clock keeps working)

/*
        |    ...    |  mas cosas de antes.
        |  *next    |   
        |  ret addr |   <- esp 

        tras pop ecx
        |    ...    |  mas cosas de antes.
        |  *next    |  <- esp 

        tras pop eax
        |    ...    |  mas cosas de antes. <- esp 

        tras push
        |    ...    |  mas cosas de antes.
        | next->eip |  <- esp ... == return address cuando se haga ret.

*/

// HAY UN POSIBLE ERROR EN SWITCH CONTEXT (EL VALOR DE EBP SE CORROMPE)


__attribute__((naked))
void switch_context(struct Proc *next) {
    __asm__ __volatile__ (        
        "pop %ecx\n"        // return address (ignore)
        // "pop %eax\n"        // eax = struct Proc* next -> tf

        "pop %gs\n"
        "pop %fs\n"
        "pop %es\n"
	    "pop %ds\n"
        // cambiarnos tf->regs

        "popa\n"

        "add $4, %esp\n" 	   // Descarto valor 'int_no'
	    "add $4, %esp\n"     // Descarto valor 'err_code'

        "sti\n"
        "iret\n"           // eip = tf->eip
    );
}

// LEGACY
__attribute__((naked))
void switch_context_kernel(struct Proc *next) {
    __asm__ __volatile__ (        
        "pop %ecx\n"        // return address (ignore)
        "pop %eax\n"        // eax = struct Proc* next -> tf

        // === Cambiar CR3 ===
        "mov 56(%eax), %edx\n"  // edx = next->pde_paddr
        "mov %edx, %cr3\n"      // Cargar el nuevo CR3

        // === Cambiar stack ===
        "mov 36(%eax), %esp\n"
        "mov 8(%eax),%ebp\n" // Restore ebp from user?

        // === Restaurar eip y registros ===
        "mov 32(%eax),%ecx\n"      // load next eip on ecx
        "push %ecx\n"// push next eip as return address

        // Restaurar registros del TrapFrame, apartir del eax.. no esp por que 
        // No deberiamos tocar esp.
        "mov 0(%eax),%edi\n"       // edi
        "mov 4(%eax),%esi\n"       // esi
        // "mov 12(%eax),%ebx\n"      // ebx (saltamos oesp)
        "mov 16(%eax),%ebx\n"      // ebx 
        "mov 20(%eax),%edx\n"      // edx
        "mov 24(%eax),%ecx\n"      // ecx
        "mov 28(%eax),%eax\n"      // eax

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
    printf("[INIT TRAPFRAME] Dir memoria trapframe: %p\n", tf);

    tf->regs.edi = 0;
    tf->regs.esi = 0;
    tf->regs.oesp = 0; 
    tf->regs.ebx = 0;
    tf->regs.edx = 0;
    tf->regs.ecx = 0;
    tf->regs.eax = 0;

    tf->ds = GD_UD | 3;
	tf->es = GD_UD | 3;
	tf->ss = GD_UD | 3;
	tf->cs = GD_UT | 3;

    tf->eflags = FL_IF;

    // Configura punto de inicio (eip)
    tf->eip = proc->pc;

    tf->esp = proc->kernel_sp; 
    tf->regs.ebp = proc->kernel_sp; // == esp inicialmente? dsps el esp crece hacia abajo
    // For now? not good? lol at least it should not be 0 or so.. should be virtual addr

    printf("[INIT TRAPFRAME] entry_point = 0x%x, esp = 0x%x\n",tf->eip, tf->esp);
}

#include "arch/logging.h"
/*
 * Actualiza el trapframe de un proceso con el contexto actual (por ejemplo, desde un trap).
 */
void update_trapframe(struct Proc *proc, FullTrapFrame *tf) {
    proc->tf.regs.eax = tf->regs.eax;
    proc->tf.regs.ebx = tf->regs.ebx;
    proc->tf.regs.ecx = tf->regs.ecx;
    proc->tf.regs.edx = tf->regs.edx;
    proc->tf.regs.esi = tf->regs.esi;
    proc->tf.regs.edi = tf->regs.edi;
    proc->tf.regs.ebp = tf->regs.ebp;
    proc->tf.eip = tf->eip;

    proc->tf.regs.oesp = tf->regs.oesp; // TODO: porlas, ver si sacar
    proc->tf.esp = tf->esp;
}
