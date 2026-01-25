#include "proc.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

#include "arch/mem_layout.h"
#include "arch/mem.h"
 
#include "string.h"
#include "stdlib.h"

// #include "ipc.h"

void free_process(struct Proc * proc){
    
    free_proc_pages(proc);
    
    proc->user_sp_start = 0;
    proc->pde_paddr = 0;

    proc->pc = 0; // Or some default one If so you want!

    proc->status = PROC_FREE; 

    
    // Trapframe reset? maybe for security reasons.. but create_process would reset it anyway!    
}

