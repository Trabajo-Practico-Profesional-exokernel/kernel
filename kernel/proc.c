#include "proc.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

#include "arch/mem_layout.h"
#include "arch/mem.h"
#include "inc/common.h"
#include "std/string.h"

// #include "ipc.h"

void free_process(struct Proc * proc){
    // TODO !!!!
    // SHOULD FREE PAGES ALLOCATED! BUT IT DOES NOT DO IT YET SINCE ALLOC PAGES IS NOT A LINKED LIST EITHER!
    // proc->kernel_sp = 0;
    proc->user_sp_start = 0;
    proc->pde_paddr = 0;

    proc->pc = 0; // Or some default one If so you want!

    proc->status = PROC_FREE; 
    
    // Trapframe reset? maybe for security reasons.. but create_process would reset it anyway!    
}