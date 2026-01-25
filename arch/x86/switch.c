#include "arch/switch.h"
#include "arch_inc/trapframe.h"
#include "arch/proc.h"
#include "std/printf.h"
#include "console/debug.h"


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
        "mov 4(%esp), %eax\n"      // EAX = next

        // Restaurar segmentos manualmente desde EAX
        "mov 0(%eax), %gs\n"
        "mov 4(%eax), %fs\n"
        "mov 8(%eax), %es\n"
        "mov 12(%eax), %ds\n"

        // Chequear Ring
        "mov 60(%eax), %ecx\n"     // CS
        "test $3, %ecx\n"
        "jz .kernel_restore\n"

    ".user_restore:\n"
        // Para usuario usamos el truco de ESP = struct
        "mov %eax, %esp\n"
        "add $16, %esp\n"          // Saltar gs..ds
        "popa\n"
        "add $8, %esp\n"           // Saltar int/err
        "iret\n"

    ".kernel_restore:\n"
        // Para kernel restauramos MANUALMENTE para no perder el control del stack
        "mov 68(%eax), %edx\n"     // EDX = tf->esp (Stack destino)
        
        // Copiar EFLAGS, CS, EIP al stack destino
        "mov 64(%eax), %ecx\n"     // EFLAGS
        "mov %ecx, -4(%edx)\n"
        "mov 60(%eax), %ecx\n"     // CS
        "mov %ecx, -8(%edx)\n"
        "mov 56(%eax), %ecx\n"     // EIP
        "mov %ecx, -12(%edx)\n"

        // Restaurar GPRs desde la memoria (EAX)
        "mov 16(%eax), %edi\n"
        "mov 20(%eax), %esi\n"
        "mov 24(%eax), %ebp\n"
        // saltamos oesp (28)
        "mov 32(%eax), %ebx\n"
        "mov 36(%eax), %edx\n"     // Restauramos EDX real
        "mov 40(%eax), %ecx\n"     // Restauramos ECX real

        // Cambiar al stack destino preparado
        "mov 68(%eax), %esp\n"     // Cargar ESP
        "sub $12, %esp\n"          // Ajustar para los 3 pushes manuales

        // Finalmente restaurar EAX
        "mov 44(%eax), %eax\n"
        
        "iret\n"
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
void init_trapframe(struct Proc *proc, vaddr_t user_sp) {
    TrapFrame *tf = &proc->tf;
    debug_printf("[INIT TRAPFRAME] Dir memoria trapframe: %p\n", tf);

    tf->regs.edi = 0;
    tf->regs.esi = 0;
    tf->regs.oesp = 0; 
    tf->regs.ebx = 0;
    tf->regs.edx = 0;
    tf->regs.ecx = 0;
    tf->regs.eax = 0;

    tf->eflags = FL_IF;

    // Configura punto de inicio (eip)
    tf->eip = proc->pc;

    tf->esp = user_sp; 
    tf->regs.ebp = user_sp; // == esp inicialmente? dsps el esp crece hacia abajo

    // Es un programa de usuario
    // Debe correr en Ring 3
    tf->ds = GD_UD | 3; 
    tf->es = GD_UD | 3;
    tf->ss = GD_UD | 3;
    tf->cs = GD_UT | 3; 
    
    // Por ahora no hay procs de kernel! y sera en otro metodo seguro!
    // if (proc->pc < 0x01000000) { 
    //     // Es código del Kernel (proc_a_entry, proc_b_entry están en ~1MB)
        // Debe correr en Ring 0
        // tf->ds = GD_KD; 
        // tf->es = GD_KD;
        // tf->ss = GD_KD;
        // tf->cs = GD_KT; 
    // }
    
    debug_printf("[INIT TF] pid=%d eip=%x -> Ring %s\n", 
           proc->pid, tf->eip, (tf->cs & 3) == 0 ? "0 (Kernel)" : "3 (User)");

    debug_printf("[INIT TRAPFRAME] entry_point = 0x%x, esp = 0x%x\n",tf->eip, tf->esp);
}

#include "arch/logging.h"
/*
 * Actualiza el trapframe de un proceso con el contexto actual (por ejemplo, desde un trap).
 */
void update_trapframe(struct Proc *proc, FullTrapFrame *tf) {
    proc->tf.regs = tf->regs;
    proc->tf.eip = tf->eip;
    proc->tf.cs = tf->cs;
    proc->tf.eflags = tf->eflags;

    if ((tf->cs & 3) == 0) { 
        // --- CORRECCIÓN CRÍTICA ---
        // Estamos en Kernel Mode. 
        // tf->regs.oesp apunta a 'int_no' en el stack.
        // Debemos saltar: int_no (4) + err_code (4) + EIP (4) + CS (4) + EFLAGS (4) = 20 bytes
        // para recuperar el ESP original antes de que ocurriera NADA de la interrupción.
        
        proc->tf.esp = tf->regs.oesp + 20; 
        
    } else {
        // En User Mode, la CPU guarda el ESP y SS viejos al final del frame.
        proc->tf.esp = tf->esp;
        proc->tf.ss = tf->ss;
    }
}
