
#include "proc.h"
#include "inc/common.h"
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem_layout.h"
#include "std/string.h"
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

extern char __trampoline_start[], __trampoline_end[];

#ifdef IS_RISC
    #define KERNEL_PERMISSIONS_ALL (PAGE_R | PAGE_W | PAGE_X)
    #define USER_PERMISSIONS_ALL (PAGE_U | PAGE_R | PAGE_W | PAGE_X)
#else
    #define KERNEL_PERMISSIONS_RW (I86_PTE_WRITABLE) // I86_PTE_PRESENT  no HACE FALTA! Ya se setea en el map_page.
    #define USER_PERMISSIONS_ALL (I86_PTE_WRITABLE | I86_PTE_USER)
#endif



void load_create_process_user(struct Proc * proc, const struct AppBinaryInfo * app_info) {    
    proc->pc = VADDR_USER_BASE; // Entry point is setted to the vaddr, here it could be the trampoline but for now is the code of prog
    
    //
    // ALLOC OF Page directory table! .. 1024 entries of 32bits, that are the configs of page tables.. allocated dynamically
    //
    
    proc->pde_paddr = alloc_pages(1);
    uint32_t *pde_table = (uint32_t *) proc->pde_paddr;
    
    printf("FOR PROC %u MAP PAGETABLE %x\n", proc->pid, proc->pde_paddr);
    
    
    //
    // ALLOC OF Process stack
    //

    proc->user_sp_start = alloc_pages(USER_STACK_PAGE_COUNT);
    printf("FOR PROC %u USER SP_START IS %x \n", proc->pid, proc->user_sp_start);



    // Mem layout is Direct mapping for users? For easier management for now.

    // First map page for page table as direct map
    map_page(pde_table, proc->pde_paddr, proc->pde_paddr, KERNEL_PERMISSIONS_ALL);
    
    //
    // Map user stack to vaddr
    // [USER_PROG_HARD_END ..USER_STACK_PAGE_COUNT .. USER_STACJ_HARD_END] ..    
    // ... to avoid having to calculate dynamic start. Also ... no page for safeguard yet. 
    //
    paddr_t paddr_sp_end = proc->user_sp_start + USER_STACK_PAGE_COUNT * PAGE_SIZE;
    offset_map_range(pde_table, proc->user_sp_start, paddr_sp_end,
            USER_PERMISSIONS_ALL, VADDR_USER_HARD_END); 



    // Map the code of the kernel so that when a syscall/trap happens there is no page fault. No permission for user. Direct map.
    printf("FOR PROC %u MAP KERNEL CODE %x to %x\n", proc->pid, get_paddr_kernel_start(), get_paddr_kernel_end());
    direct_map_range(pde_table, 
            get_paddr_kernel_start(),
            get_paddr_kernel_end(),
            KERNEL_PERMISSIONS_ALL
    );

    // Map trampoline so that It can be accesed from user space... 
    direct_map_range(pde_table, 
            (paddr_t) __trampoline_start,
            (paddr_t) __trampoline_end,
            USER_PERMISSIONS_ALL
    );



    // Map the code of the user program/binary... loading it from memory
    for (uint32_t off = 0; off < app_info->size; off += PAGE_SIZE) {
        paddr_t page = alloc_pages(1); // Alloc pages throws PANIC ALREADY!
        if(off == 0){
            printf("PADDR START OF PROCESS 0x%x\n",page);
        }

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


    // Sets on the trapframe th pc to the right vl
    // Sets sp to the value.. and resets registers
    init_trapframe(proc);

    // Setup simple parameters?
    struct TrapFrame * proc_tf = &proc->tf;
    
    SET_SYSCALL_RET0(proc_tf, 12);
    SET_SYSCALL_RET1(proc_tf, 21);

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