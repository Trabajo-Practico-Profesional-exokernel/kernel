#include "arch/trap.h"
#include "arch/proc.h"

#include "inc/common.h"

void init_trap(){

    WRITE_CSR(stvec, (uint32_t) trap_entry); // riscv5 , set in case of interruption trap entry to be exec 

    printf("Initing superviser mode/enable clock?! \n");
    uint32_t mcount = READ_CSR(mideleg);
    printf("mideleg vl %u \n", mcount);

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
        printf("log trap from userspace scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
        
        //uintptr_t new_pc = syscall_dispatch(sp);  // dispatch syscall?
        //WRITE_CSR(user_pc, new_pc); // Redirect execution

        user_pc += 4;  // Skip ins
        WRITE_CSR(sepc, user_pc);        
    } else if(scause == 2) {
        // Just for testing purpose? skip this instruction
        printf("ignore trap illegal ins scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
        user_pc += 4;  // Skip illegal instruction
        WRITE_CSR(sepc, user_pc);
    } else if(IS_CLOCK_INTERRUPT(scause)) {
        
        // printf("Clock interrupt ins scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
        SET_NEXT_INTERRUPT(DELAY_INTERRUPT);
        sched_yield(tf);
    } else {
        PANIC("unexpected trap scause=%u, stval=%x, sepc=%x\n", scause, stval, user_pc);
    }
    //printf("log trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}