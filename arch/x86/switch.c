#include "arch/switch.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"
#include "inc/common.h"

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

static inline void load_cr3(uint32_t paddr) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(paddr) : "memory");
}

void switch_to_page_table(struct Proc *p) {
    if (!p || !p->page_table) return;
    uint32_t cr3 = (uint32_t)(p->page_table->paddr & 0xFFFFF000);
    load_cr3(cr3);
}


// cuando tengamos espacio de usuario, cambiar por iret que restaura eip y esp
// TODO: add switching of CR3
__attribute__((naked))
void switch_context(struct Proc *next) {
    __asm__ __volatile__ (        
        "pop %ecx\n"        // return address (ignore)
        "pop %eax\n"        // eax = struct Proc* next -> tf

        // === Cambiar CR3 ===
        // "mov 56(%eax), %edx\n"  // edx = next->page_table
        // "mov 4(%edx), %edx\n"   // edx = (next->page_table)->paddr
        // "mov %edx, %cr3\n"      // Cargar el nuevo CR3

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
        "mov 12(%eax),%ebx\n"      // ebx (saltamos oesp)
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

    tf->edi = 0;
    tf->esi = 0;
    tf->oesp = 0; 
    tf->ebx = 0;
    tf->edx = 0;
    tf->ecx = 0;
    tf->eax = 0;

    // Configura punto de inicio (eip)
    tf->eip = proc->pc;

    tf->esp = proc->kernel_sp; 
    tf->ebp = proc->kernel_sp; // == esp inicialmente? dsps el esp crece hacia abajo
    // For now? not good? lol at least it should not be 0 or so.. should be virtual addr

    printf("[INIT TRAPFRAME] entry_point = 0x%x, esp = 0x%x\n",tf->eip, tf->esp);
}

#include "arch/logging.h"
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