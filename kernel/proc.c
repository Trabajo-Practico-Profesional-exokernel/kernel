#include "proc.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

#include "arch/mem_layout.h"
#include "arch/mem.h"
#include "inc/common.h"

//#include "arch/logging.h"

#define KERNEL_PERMISSIONS_ALL PAGE_R | PAGE_W| PAGE_X


void create_process(struct Proc * proc, uint32_t pc) { // pc == entry point == start instruction
    // Save initial pc on proc.
    proc->pc = pc;
    
    // For now kernel stack of process... is on the proc struct itself! xv6 does it in a page a virtual memory.. for the future
    vaddr_t sp_base = alloc_pages(KERN_STACK_PAGES);
    proc->kernel_sp =  sp_base + KERN_STACK_PAGES * PAGE_SIZE;

    // Stack callee-saved registers. These register values will be restored in
    // the first context switch in switch_context. ... init registers basically?
    // After proc->kernel_sp and proc->pc setted up so that they can be included on trapframe if needed
    init_trapframe(proc);

    #ifdef IS_RISC
    // Initialize memory/pagetables
    printf("NO DEBERIA ENTRAR ACA X86!!!!\n");

    // TODO: poner en una funcion create_page_table o algo
    paddr_t page_table_addr = alloc_pages(1);
    uint32_t *page_table = (uint32_t *) page_table_addr;

    gen_pt_t pt = {
        .root = page_table,
        .paddr = page_table_addr, //para satp!
    };
    
    printf("FOR PROC %u MAP PAGETABLE %x\n", proc->pid, page_table_addr);
    // First map page for page table as direct map
    map_page(*pt,page_table_addr, page_table_addr, KERNEL_PERMISSIONS_ALL);

    printf("FOR PROC %u MAP KERNEL STACK %x to %x\n", proc->pid, sp_base, proc->kernel_sp);
    // Also map kernel stack
    direct_map_range(*pt, 
            sp_base, proc->kernel_sp, KERNEL_PERMISSIONS_ALL);

    printf("FOR PROC %u MAP KERNEL CODE %x to %x\n", proc->pid, get_paddr_kernel_start(), get_paddr_kernel_end());
    // Map base kernel code pages, does not include any allocated pages, like the page_table_addr
    direct_map_range(*pt, 
            get_paddr_kernel_start(),
            get_paddr_kernel_end(),
            KERNEL_PERMISSIONS_ALL
    );

    
 
    proc->page_table = pt;  // TODO: esto va a romper en mil lados... corregirlo en otra rama
    #endif

    #ifdef IS_X86

    gen_pt_t *gen_pt = get_gen_table();

    // Mapear el stack del kernel
    printf("FOR PROC %u MAP KERNEL STACK %x to %x\n", proc->pid, sp_base, proc->kernel_sp);
    direct_map_range(gen_pt, sp_base, proc->kernel_sp, KERNEL_PERMISSIONS_ALL);

    // Mapear los code/data segments del kernel 
    printf("FOR PROC %u MAP KERNEL CODE %x to %x\n", proc->pid, get_paddr_kernel_start(), get_paddr_kernel_end());
    direct_map_range(gen_pt, get_paddr_kernel_start(), get_paddr_kernel_end(), KERNEL_PERMISSIONS_ALL);

    proc->page_table = gen_pt;
    printf("[DBG] DESPUES `proc->page_table`:  gen_pt=%x &gen_pt=%x paddr=%x\n",
       (uint32_t)gen_pt,
       (uint32_t)&gen_pt,
       (uint32_t)gen_pt->paddr);

    #endif

    proc->status = PROC_RUNNABLE;
}

void create_process_user(struct Proc * proc, uint32_t proc_entry,
    paddr_t user_space_start, paddr_t user_space_end) {

    create_process(proc, proc_entry);//((uint32_t) user_entry);

    direct_map_range(proc->page_table,
        (paddr_t)user_space_start,
        (paddr_t)user_space_end,
        PAGE_U | PAGE_R | PAGE_W | PAGE_X // User space page.
        );
}


void load_create_process_user(struct Proc * proc, const void *image, size_t image_size) {
    create_process(proc, VADDR_USER_BASE);//((uint32_t) user_entry);

    // Map user app instruction pages.. i.e load to memory the process
    for (uint32_t off = 0; off < image_size; off += PAGE_SIZE) {
        paddr_t page = alloc_pages(1);

        // Handle the case where the data to be copied is smaller than the
        // page size.
        size_t remaining = image_size - off;
        size_t copy_size = PAGE_SIZE <= remaining ? PAGE_SIZE : remaining;

        // Fill and map the page.
        memcpy((void *) page, image + off, copy_size);
        map_page(proc->page_table, VADDR_USER_BASE + off, page,
                 PAGE_U | PAGE_R | PAGE_W | PAGE_X);
    }
}