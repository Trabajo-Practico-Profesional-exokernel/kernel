
#include "arch_inc/trap_constants.h"
#include "arch_inc/riscv5_trap.h"

#include "arch/trap.h"
#include "arch/proc.h"

#include "inc/common.h"

#include "arch/trap_handling.h"

// Assume its defined somewhere
void clock_yield(FullTrapFrame *tf, uintptr_t pc);

/*
Initing superviser mode/enable clock?! 
ignore trap illegal ins scause=00000002, stval=30402573, sepc=80200072 csrr a0, mie
ignore trap illegal ins scause=00000002, stval=30451073, sepc=8020007a csrw mie, a0
ignore trap illegal ins scause=00000002, stval=30a02573, sepc=8020007e csrr a0, menvcfg
ignore trap illegal ins scause=00000002, stval=30a51073, sepc=80200082 csrw menvcfg, a0
ignore trap illegal ins scause=00000002, stval=30602573, sepc=80200086 csrr a0, mcounteren
ignore trap illegal ins scause=00000002, stval=30651073, sepc=8020008e csrw mcounteren, a0


*/

#define CLOCK_INTERRUPT_SCAUSE 2147483653//80000005

#define SCAUSE_INT_MASK     (1UL << 31)
#define SCAUSE_CAUSE_MASK   (~SCAUSE_INT_MASK)
#define SCAUSE_SUPERVISOR_TIMER 5
#define DELAY_INTERRUPT 1000000

// This is a supervisor timer interrupt
#define IS_CLOCK_INTERRUPT(value) \
    (value & SCAUSE_INT_MASK) && ((value & SCAUSE_CAUSE_MASK) == SCAUSE_SUPERVISOR_TIMER) \


#define SET_NEXT_INTERRUPT(delay) \
    w_stimecmp(r_time() + delay);

void init_trap(){

    WRITE_CSR(stvec, (uint32_t) trap_entry); // riscv5 , set in case of interruption trap entry to be exec 

    debug_printf("Initing superviser mode/enable clock?! \n");
    //uint32_t mcount = READ_CSR(mideleg);
    //printf("mideleg vl %u \n", mcount);

    WRITE_CSR(sie, READ_CSR(sie) | SIE_STIE);
    WRITE_CSR(sstatus, READ_CSR(sstatus) | (1 << 1)); // SSTATUS_SIE
    
    SET_NEXT_INTERRUPT(DELAY_INTERRUPT);


    /*
    // For baremetal bootloader in M-Mode , for the future!
    // enable supervisor-mode timer interrupts.
    WRITE_CSR(mie, READ_CSR(mie) | MIE_STIE);

    // enable the sstc extension (i.e. stimecmp).
    w_menvcfg(r_menvcfg() | (1L << 63)); 
      
    // allow supervisor to use stimecmp and time.
    w_mcounteren(r_mcounteren() | 2);
      
    // ask for the very first timer interrupt.
    w_stimecmp(r_time() + 1000000);
    */

}

/*
* TRAP handling entry
* Essentially it saves the stack pointer on sscratch and after restores it on a0? and calls handle trap
* the rest is backing up registers? on stack it seems.
*/
__attribute__((naked))
__attribute__((aligned(4)))
void trap_entry(void) {
    __asm__ __volatile__(
        // We assume that on sscratch is the kernel stack pointer for the curr proc
        // and that the curr sp is the sp from userspace
        "csrrw sp, sscratch, sp\n"   // Swap user sp into sscratch, use kernel sp is on sscratch.. now in sp
        "addi sp, sp, -4 * 31\n"
        "sw ra,  4 * 0(sp)\n"
        "sw gp,  4 * 1(sp)\n"
        "sw tp,  4 * 2(sp)\n"
        "sw t0,  4 * 3(sp)\n"
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"
        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"
        "sw t5,  4 * 8(sp)\n"
        "sw t6,  4 * 9(sp)\n"
        "sw a0,  4 * 10(sp)\n"
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"
        "sw a5,  4 * 15(sp)\n"
        "sw a6,  4 * 16(sp)\n"
        "sw a7,  4 * 17(sp)\n"
        "sw s0,  4 * 18(sp)\n"
        "sw s1,  4 * 19(sp)\n"
        "sw s2,  4 * 20(sp)\n"
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"

        "csrr a0, sscratch\n"
        "sw a0, 4 * 30(sp)\n"

        "mv a0, sp\n"
        "call handle_trap\n"

        "lw ra,  4 * 0(sp)\n"
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "addi sp, sp, 4 * 31\n" // removed already used kernel stack space.
        "csrrw sp, sscratch, sp\n"// put on sscratch the kernel stack once again, and the user stack on sp
        "sret\n"
    );
}


void handle_trap(FullTrapFrame *tf) {
    uintptr_t scause = READ_CSR(scause); // This reads could come on the trapframe like xv6 does tbh
    uintptr_t stval = READ_CSR(stval);
    uintptr_t user_pc = READ_CSR(sepc);

    // ** geppeto**
    // Trap cause codes: RISC-V Privileged Spec
    // Example (not exhaustive):
    // 8  = Environment call from U-mode (ecall)
    // 9  = Environment call from S-mode
    // 12 = Instruction page fault
    // 2  = Illegal instruction
    // ...

    if (scause == 8) {
        // Syscall from user mode
        // Example: dispatch to syscall handler
        //printf("log trap from userspace scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
        
        //uintptr_t new_pc = syscall_dispatch(sp);  // dispatch syscall?
        //WRITE_CSR(user_pc, new_pc); // Redirect execution
        user_pc = handle_syscall(tf, user_pc);
        //user_pc += 4;  // Skip ins
        WRITE_CSR(sepc, user_pc);      

    } else if(scause == 2) {
        // Just for testing purpose? skip this instruction
        //printf("ignore trap illegal ins scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
        PANIC("DO NOT IGNORE trap illegal ins scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
        
        user_pc += 4;  // Skip illegal instruction
        WRITE_CSR(sepc, user_pc);
    } else if(IS_CLOCK_INTERRUPT(scause)) {
        // debug_printf("Clock interrupt ins scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
        SET_NEXT_INTERRUPT(DELAY_INTERRUPT);
        clock_yield(tf, user_pc);
    } else {
        PANIC("unexpected trap scause=%u, stval=%x, sepc=%x\n", scause, stval, user_pc);
    }
    //printf("log trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}
