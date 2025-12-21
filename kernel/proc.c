#include "proc.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

#include "arch/mem_layout.h"
#include "arch/mem.h"
#include "inc/common.h"
#include "std/string.h"

// #include "ipc.h"

#ifdef IS_RISC
#else
    #include "paging.h" 
#endif

#ifdef IS_RISC
#else
    #define KERNEL_PERMISSIONS_RW (I86_PTE_PRESENT | I86_PTE_WRITABLE)
    #define USER_PERMISSIONS_ALL (I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER)
#endif


void free_process(struct Proc * proc){
    // TODO !!!!
    // SHOULD FREE PAGES ALLOCATED! BUT IT DOES NOT DO IT YET SINCE ALLOC PAGES IS NOT A LINKED LIST EITHER!
    proc->kernel_sp = 0;
    proc->pde_paddr = 0;

    proc->pc = 0; // Or some default one If so you want!

    proc->status = PROC_FREE; 
    
    // Trapframe reset? maybe for security reasons.. but create_process would reset it anyway!    
}


void create_process(struct Proc * proc, uint32_t pc) { // pc == entry point == start instruction
    // Save initial pc on proc.
    printf("#####################################\n"); 
    proc->pc = pc;



    //
    // ALLOC OF Process stack
    //

    paddr_t sp_base = alloc_pages(KERN_STACK_PAGES);
    printf("FOR PROC %u SP_BASE IS %x \n", proc->pid, sp_base);
    proc->kernel_sp =  sp_base + KERN_STACK_PAGES * PAGE_SIZE;

    printf("FOR PROC %u MAP KERNEL STACK %x to %x\n", proc->pid, sp_base, proc->kernel_sp);

    // Stack callee-saved registers. These register values will be restored in
    // the first context switch in switch_context. ... init registers basically?
    // After proc->kernel_sp and proc->pc setted up so that they can be included on trapframe if needed
    init_trapframe(proc);

    #ifdef IS_RISC    
    #else //x86

    paddr_t pde_paddr = alloc_pages(1);
    if (!pde_paddr) PANIC("create_process: out of memory (page_dir)");
    memset((void*)pde_paddr, 0, PAGE_SIZE);

    struct pdirectory *new_dir = (struct pdirectory*) pde_paddr; 
    struct pdirectory *kernel_dir = vm_manager_get_directory(); // Obtiene el PD actual (del kernel)

    if (kernel_dir) { 
        new_dir->m_entries[PAGE_DIRECTORY_INDEX(0x00000000)] = kernel_dir->m_entries[PAGE_DIRECTORY_INDEX(0x00000000)];

        for (int i = PAGE_DIRECTORY_INDEX(VADDR_KERNEL_BASE); i < 1024; i++) {
            new_dir->m_entries[i] = kernel_dir->m_entries[i];
        }
    } else {
        PANIC("create_process: kernel_dir es NULL");
    }

    printf("FOR PROC %u MAP KERNEL STACK %x to %x\n", proc->pid, sp_base, proc->kernel_sp);
    vaddr_t v_stack = sp_base;
    while (v_stack < proc->kernel_sp) {
        map_page(new_dir, v_stack, v_stack, KERNEL_PERMISSIONS_RW); 
        v_stack += PAGE_SIZE;
    }

    proc->pde_paddr = pde_paddr; 
    printf("[DBG] `proc->pde_paddr` CREADO: paddr=%x\n",
       (uint32_t)proc->pde_paddr);
    #endif

    proc->status = PROC_RUNNABLE;
    printf("##################################### PROCESO LISTO\n"); 
}