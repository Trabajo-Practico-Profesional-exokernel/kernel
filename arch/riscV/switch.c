#include "arch/switch.h"
#include "arch/mem_layout.h"
 

/*
sret inspects two bits in sstatus:
SPP (bit 8): Determines which mode to return to
SPIE (bit 5): Gets copied to SIE (enables supervisor interrupts after returning)
*/
// sstatus flags for use with sret
#define SSTATUS_SPP_KERNEL   (1 << 8) // SPP = 1: return to Supervisor Mode
#define SSTATUS_SPP_USER     (0 << 8) // SPP = 0: return to User Mode (just for clarity)
#define SSTATUS_SPIE         (1 << 5) // SPIE = 1: enable interrupts after sret

#define SSTATUS_KERNEL       (SSTATUS_SPP_KERNEL | SSTATUS_SPIE)
#define SSTATUS_USER         (SSTATUS_SPP_USER | SSTATUS_SPIE)


//[next_tf] "r" (next)
//, [sepc_ins] "r" ()         

//in riscv-5 ... *prev would be at register a0, *next at a1.
// in riscv-5 .. when entering on switch context the proc curr pc is at proc->pc
// proc->pc is right after proc->tf that is at the start so..
// sepc should be setted to the vl right after the sp.
__attribute__((naked)) 
void switch_context(struct Proc* next) {
    __asm__ __volatile__(       
        // Restore registers for *next
        "lw ra,  0  * 4(a0)\n"  // Remember a0 == *next, bla bla ra is pointed!
        "lw s0,  1  * 4(a0)\n"
        "lw s1,  2  * 4(a0)\n"
        "lw s2,  3  * 4(a0)\n"
        "lw s3,  4  * 4(a0)\n"
        "lw s4,  5  * 4(a0)\n"
        "lw s5,  6  * 4(a0)\n"
        "lw s6,  7  * 4(a0)\n"
        "lw s7,  8  * 4(a0)\n"
        "lw s8,  9  * 4(a0)\n"
        "lw s9,  10 * 4(a0)\n"
        "lw s10, 11 * 4(a0)\n"
        "lw s11, 12 * 4(a0)\n"

        // Lets reserve a7 for kernel usage i.e it is not restored/saved for users , at least for now
        "addi a7, a0, 0\n" // We will restore a0 so needed to back it up to a7 that is reserved for kernel usage. By design.
        
        "lw sp, 13 * 4(a7)\n" // Switch stack pointer (sp) here
        "lw a0, 14 * 4(a7)\n" 
        "lw a1, 15 * 4(a7)\n"
        "lw a2, 16 * 4(a7)\n"
        "lw a3, 17 * 4(a7)\n"

        "lw a6, 18 * 4(a7)\n" // a6 could be restored afterwards but for now not added to tf of user proc.
        "csrw sepc, a6\n" // Set sepc, where the sret jumps back to... for now to the proc->pc no trampoline
        
        "li a7, %[sstatus]\n" // Set a7 value to sttatus used, now next proc param is not used anymore
        "csrw sstatus, a7\n"

        "sret\n"
        :
        : [sstatus] "i" (SSTATUS_USER)//(SSTATUS_KERNEL)
        : "a0", "a1"
    );
}



/// NOT USED , but have to check what purpose exactly it had, since could be for returning from traps?
__attribute__((naked)) void user_entry(void) {
    __asm__ __volatile__(
        "csrw sepc, %[sepc]\n"
        "csrw sstatus, %[sstatus]\n"
        "sret\n"
        :
        : [sepc] "r" (VADDR_USER_BASE),
          [sstatus] "r" (SSTATUS_USER)
    );
}


void sleep(int delay) {
    for (int i = 0; i < delay; i++)
        __asm__ __volatile__("nop"); // do nothing
}

void init_trapframe(struct Proc * proc, vaddr_t user_sp){
    proc->tf.s11 = 0;
    proc->tf.s10 = 0;
    proc->tf.s9 = 0;
    proc->tf.s8 = 0;
    proc->tf.s7 = 0;
    proc->tf.s6 = 0;
    proc->tf.s5 = 0;
    proc->tf.s4 = 0;
    proc->tf.s3 = 0;
    proc->tf.s2 = 0;
    proc->tf.s1 = 0;
    proc->tf.s0 = 0;

    proc->tf.sp = user_sp;

    proc->tf.ra = proc->pc; // For now ra setted to proc initial pc?    

    // Registers for params/return values
    proc->tf.a0 = 0;
    proc->tf.a1 = 0;
    proc->tf.a2 = 0;
    proc->tf.a3 = 0;

}


void update_trapframe(struct Proc *proc, FullTrapFrame *tf){
    proc->tf.s11 = tf->s11;
    proc->tf.s10 = tf->s10;
    proc->tf.s9 = tf->s9;
    proc->tf.s8 = tf->s8;
    proc->tf.s7 = tf->s7;
    proc->tf.s6 = tf->s6;
    proc->tf.s5 = tf->s5;
    proc->tf.s4 = tf->s4;
    proc->tf.s3 = tf->s3;
    proc->tf.s2 = tf->s2;
    proc->tf.s1 = tf->s1;
    proc->tf.s0 = tf->s0;
    proc->tf.ra = tf->ra;

    // SET USER SP! Fulltf has the user one!
    proc->tf.sp = tf->sp;

    // Registers for params/return values
    proc->tf.a0 = tf->a0;
    proc->tf.a1 = tf->a1;
    proc->tf.a2 = tf->a2;
    proc->tf.a3 = tf->a3;
}




