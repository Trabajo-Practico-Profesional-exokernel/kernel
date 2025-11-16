#include "proc.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

#include "arch/mem_layout.h"
#include "arch/mem.h"
#include "inc/common.h"
#include "std/string.h"

#ifdef IS_RISC
#else
    #include "paging.h" 
#endif

// TODO: ver si se puede unificar
#ifdef IS_RISC
    #define KERNEL_PERMISSIONS_ALL (PAGE_R | PAGE_W | PAGE_X)
    #define USER_PERMISSIONS_ALL (PAGE_U | PAGE_R | PAGE_W | PAGE_X)
#else
    #define KERNEL_PERMISSIONS_RW (I86_PTE_PRESENT | I86_PTE_WRITABLE)
    #define USER_PERMISSIONS_ALL (I86_PTE_PRESENT | I86_PTE_WRITABLE | I86_PTE_USER)
#endif




void create_process(struct Proc * proc, uint32_t pc) { // pc == entry point == start instruction
    // Save initial pc on proc.
    printf("#####################################\n"); 
    proc->pc = pc;
    
    // For now kernel stack of process... is on the proc struct itself! xv6 does it in a page a virtual memory.. for the future
    vaddr_t sp_base = alloc_pages(KERN_STACK_PAGES);
    printf("FOR PROC %u SP_BASE IS %x \n", proc->pid, sp_base);
    proc->kernel_sp =  sp_base + KERN_STACK_PAGES * PAGE_SIZE;

    // Stack callee-saved registers. These register values will be restored in
    // the first context switch in switch_context. ... init registers basically?
    // After proc->kernel_sp and proc->pc setted up so that they can be included on trapframe if needed
    init_trapframe(proc);

    #ifdef IS_RISC
    // Initialize memory/pagetables
    paddr_t page_table_addr = alloc_pages(1);
    uint32_t *page_table = (uint32_t *) page_table_addr;
    
    printf("FOR PROC %u MAP PAGETABLE %x\n", proc->pid, page_table_addr);
    // First map page for page table as direct map
    map_page(page_table,page_table_addr, page_table_addr, KERNEL_PERMISSIONS_ALL);

    printf("FOR PROC %u MAP KERNEL STACK %x to %x\n", proc->pid, sp_base, proc->kernel_sp);
    // Also map kernel stack
    direct_map_range(page_table, 
            sp_base, proc->kernel_sp, KERNEL_PERMISSIONS_ALL);

    printf("FOR PROC %u MAP KERNEL CODE %x to %x\n", proc->pid, get_paddr_kernel_start(), get_paddr_kernel_end());
    // Map base kernel code pages, does not include any allocated pages, like the page_table_addr
    direct_map_range(page_table, 
            get_paddr_kernel_start(),
            get_paddr_kernel_end(),
            KERNEL_PERMISSIONS_ALL
    );

    proc->pde_paddr = page_table_addr;

    #else //x86

    paddr_t pde_paddr = alloc_pages(1);
    if (!pde_paddr) PANIC("create_process: out of memory (page_dir)");
    memset((void*)pde_paddr, 0, PAGE_SIZE);

    // 2. Obtener punteros virtuales al PD nuevo y al del kernel
    // (Asumimos mapeo 1:1 de la RAM física donde se alojan los PDs)
    struct pdirectory *new_dir = (struct pdirectory*) pde_paddr; 
    struct pdirectory *kernel_dir = vm_manager_get_directory(); // Obtiene el PD actual (del kernel)

    // 3. Copiar los mapeos del kernel
    if (kernel_dir) { 

        // Copiar el mapeo de identidad (0x0-0x400000)
        new_dir->m_entries[PAGE_DIRECTORY_INDEX(0x00000000)] = kernel_dir->m_entries[PAGE_DIRECTORY_INDEX(0x00000000)];

        for (int i = PAGE_DIRECTORY_INDEX(0xC0000000); i < 1024; i++) {
            new_dir->m_entries[i] = kernel_dir->m_entries[i];
        }
    } else {
        PANIC("create_process: kernel_dir es NULL");
    }

    // 4. Mapear el stack de kernel del *nuevo* proceso en su *propio* PD
    printf("FOR PROC %u MAP KERNEL STACK %x to %x\n", proc->pid, sp_base, proc->kernel_sp);
    vaddr_t v_stack = sp_base;
    while (v_stack < proc->kernel_sp) {
        // Mapeo 1:1 del stack (v_stack -> v_stack)
        vmmngr_map_page_to_dir(new_dir, v_stack, v_stack, KERNEL_PERMISSIONS_RW); 
        v_stack += PAGE_SIZE;
    }

    // 5. Guardar la dirección física del Page Directory
    proc->pde_paddr = pde_paddr; 
    printf("[DBG] `proc->pde_paddr` CREADO: paddr=%x\n",
       (uint32_t)proc->pde_paddr);
    #endif

    proc->status = PROC_RUNNABLE;
    printf("##################################### PROCESO LISTO\n"); 
}

void create_process_user(struct Proc * proc, uint32_t proc_entry,
    paddr_t user_space_start, paddr_t user_space_end) {

    create_process(proc, proc_entry);//((uint32_t) user_entry);

    // TODO: una vez que globalicemos lo de memoria virtual, refactorear esto
    #ifdef IS_RISC
    uint32_t *page_table = (uint32_t *) proc->pde_paddr;
    direct_map_range(page_table,
        (paddr_t)user_space_start,
        (paddr_t)user_space_end,
        USER_PERMISSIONS_ALL
        );
    #else
    struct pdirectory *new_dir = (struct pdirectory*) proc->pde_paddr;
    vaddr_t v_user = (vaddr_t)user_space_start;
    paddr_t p_user = (paddr_t)user_space_start;

    while (p_user < user_space_end) {
        // Mapeo 1:1 (p_user -> v_user, que son iguales)
        vmmngr_map_page_to_dir(new_dir, p_user, v_user, USER_PERMISSIONS_ALL);
        v_user += PAGE_SIZE;
        p_user += PAGE_SIZE;
    }
    #endif
}

void load_create_process_user(struct Proc * proc, const struct AppBinaryInfo * app_info) {    
    create_process(proc, VADDR_USER_BASE);

    #ifdef IS_RISC
    uint32_t *page_table = (uint32_t *) proc->pde_paddr;
    #else
    struct pdirectory *new_dir = (struct pdirectory*) proc->pde_paddr;
    #endif

    // Mapear paginas de usuario y copiar la imagen
    for (uint32_t off = 0; off < app_info->size; off += PAGE_SIZE) {
        paddr_t page = alloc_pages(1);
        if (page == 0) {
            PANIC("load_create_process_user: out of memory");
        }

        // Handle the case where the data to be copied is smaller than the page size.
        size_t remaining = app_info->size - off;
        size_t copy_size = (PAGE_SIZE <= remaining) ? PAGE_SIZE : remaining;

        // Copiar los datos a la página física recién asignada
        memcpy((void *) page, app_info->start + off, copy_size);

        // Mapear la página física en el espacio de direcciones virtual del proceso
        #ifdef IS_RISC
        map_page(page_table, VADDR_USER_BASE + off, page,
                 USER_PERMISSIONS_ALL);
        #else
        vmmngr_map_page_to_dir(new_dir, page, VADDR_USER_BASE + off, 
                 USER_PERMISSIONS_ALL);
        #endif
    }
}
