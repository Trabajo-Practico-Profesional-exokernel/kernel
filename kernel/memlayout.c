
#include "proc.h"
#include "inc/common.h"
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem_layout.h"
#include "std/string.h"

extern char __trampoline_start[], __trampoline_end[];

#ifdef IS_RISC
    #define KERNEL_PERMISSIONS_ALL (PAGE_R | PAGE_W | PAGE_X)
    #define USER_PERMISSIONS_ALL (PAGE_U | PAGE_R | PAGE_W | PAGE_X)
#else
    #define KERNEL_PERMISSIONS_RW (I86_PTE_WRITABLE) // I86_PTE_PRESENT  no HACE FALTA! Ya se setea en el map_page.
    #define USER_PERMISSIONS_ALL (I86_PTE_WRITABLE | I86_PTE_USER)
#endif



void load_create_process_user(struct Proc * proc, const struct AppBinaryInfo * app_info) {    
    create_process(proc, VADDR_USER_BASE);

    //
    // ALLOC OF Page directory table! .. 1024 entries of 32bits, that are the configs of page tables.. allocated dynamically
    //
    
    proc->pde_paddr = alloc_pages(1);
    uint32_t *pde_table = (uint32_t *) proc->pde_paddr;
    
    printf("FOR PROC %u MAP PAGETABLE %x\n", proc->pid, proc->pde_paddr);

    // Mem layout is Direct mapping for users? For easier management for now.
    // First map page for page table as direct map
    map_page(pde_table, proc->pde_paddr, proc->pde_paddr, KERNEL_PERMISSIONS_ALL);


    //
    // Map kern stack also direct map for now
    //

    paddr_t sp_base = proc->kernel_sp - KERN_STACK_PAGES * PAGE_SIZE;
    direct_map_range(pde_table, 
            sp_base, proc->kernel_sp, KERNEL_PERMISSIONS_ALL);
    
    // Map the code of the kernel so that when a syscall/trap happens there is no page fault. No permission for user. Direct map.
    
    printf("FOR PROC %u MAP KERNEL CODE %x to %x\n", proc->pid, get_paddr_kernel_start(), get_paddr_kernel_end());
    direct_map_range(pde_table, 
            get_paddr_kernel_start(),
            get_paddr_kernel_end(),
            KERNEL_PERMISSIONS_ALL
    );

    
    // Map the code of the user program/binary... loading it from memory
    for (uint32_t off = 0; off < app_info->size; off += PAGE_SIZE) {
        paddr_t page = alloc_pages(1); // Alloc pages throws PANIC ALREADY!


        // Handle the case where the data to be copied is smaller than the page size.
        size_t remaining = app_info->size - off;
        size_t copy_size = (PAGE_SIZE <= remaining) ? PAGE_SIZE : remaining;

        // Copiar los datos a la página física recién asignada
        // A futuro... no muy lejano.... esto no seria con memcpy, sino accediendo a disco, asi no se carga a memoria todos los programas.
        memcpy((void *) page, app_info->start + off, copy_size);

        // Map the loaded code to the VADDR of the user programs
        map_page(pde_table, VADDR_USER_BASE + off, page,
                 USER_PERMISSIONS_ALL);
    }
}

// For now no extra mapping needed.
void load_create_process_kernel(struct Proc * proc, uint32_t proc_entry){
    create_process(proc, proc_entry);
    // proc->pde_table = kernel_page_table; // Should just set pages == to kernel ones
} 


// void create_process_user(struct Proc * proc, uint32_t proc_entry,
//     paddr_t user_space_start, paddr_t user_space_end) {

//     create_process(proc, proc_entry);//((uint32_t) user_entry);

//     // TODO: una vez que globalicemos lo de memoria virtual, refactorear esto
//     #ifdef IS_RISC
//     uint32_t *page_table = (uint32_t *) proc->pde_paddr;
//     direct_map_range(page_table,
//         (paddr_t)user_space_start,
//         (paddr_t)user_space_end,
//         USER_PERMISSIONS_ALL
//         );
//     #else
//     struct pdirectory *new_dir = (struct pdirectory*) proc->pde_paddr;
//     vaddr_t v_user = (vaddr_t)user_space_start;
//     paddr_t p_user = (paddr_t)user_space_start;

//     while (p_user < user_space_end) {
//         // Mapeo 1:1 (p_user -> v_user, que son iguales)
//         map_page(new_dir, p_user, v_user, USER_PERMISSIONS_ALL);
//         v_user += PAGE_SIZE;
//         p_user += PAGE_SIZE;
//     }
//     #endif
// }