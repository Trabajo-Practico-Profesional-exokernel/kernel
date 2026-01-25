
#include "proc.h"
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem_layout.h"
#include "std/string.h"
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.
#include "constants.h"
#include "proc_syscalls.h"
#include "user_pages_alloc.h" 
#include "std/printf.h"

extern char __trampoline_start[], __trampoline_end[];

int copy_argv_pointers_from_user(struct Proc * proc, paddr_t* argv_pointers, vaddr_t vaddr_argv){
    paddr_t src_argv_paddr = get_paddr_for(
                                    (uint32_t *) proc->pde_paddr,
                                    vaddr_argv);
    
    if (src_argv_paddr == 0){
        printf("Invalid vaddr for argv error!!\n");
        return -2;
    }
    vaddr_t* src_argv = (vaddr_t*) src_argv_paddr;
    int argc;

    for(argc = 0; src_argv[argc]; argc++) { // While argv[ind] != 0
        if(argc >= MAXARG) {
            printf("MORE THAN MAX PARAMS!\n");
            return -1;
        }

        vaddr_t vaddr_arg = src_argv[argc];
        
        paddr_t paddr_arg = get_paddr_for(
                                        (uint32_t *) proc->pde_paddr,
                                        vaddr_arg);
        if (paddr_arg == 0){
            printf("Invalid vaddr for arg error!!\n");
            return -2;
        }

        // Just for validation... by the way.. this is wrong since arg parameter could be in multiple pages
        // that are not contiguous.. not now though since kalloc does not exist.
        size_t arg_len = strlen((char* ) paddr_arg) + 1; 
        
        if (arg_len > MAX_ARG_LEN){
            printf("ARG LONGER THAN ALLOWED!\n");
            return -1;            
        }

        debug_printf("MAPPED PARAM FOR PROGRAM pointer at %x!\n", paddr_arg);
        
        argv_pointers[argc] = paddr_arg;
    }
    
    argv_pointers[argc] = 0;

    return argc;
}






int set_init_parameters_for_proc(struct Proc * proc, char ** argv, paddr_t* sp_out){
    // Copy arguments to the stack of the proc
    // In the future it could be we use instead dynamically allocated pages
    // Not needed for now.
    paddr_t paddr_sp_end= proc->user_sp_start + USER_STACK_PAGE_COUNT * PAGE_SIZE; // Start at the stack top
    paddr_t sp = paddr_sp_end;
    
    uint32_t argc;
    paddr_t argv_pointers[MAXARG];
    for(argc = 0; argv[argc]; argc++) { // While argv[ind] != 0

        if(argc >= MAXARG) {
            debug_printf("MORE THAN MAX PARAMS!\n");
            return -1;
        }
        debug_printf("Pushing ARG at 0x%x to stack ", argv[argc]);
        debug_printf("'%s'!\n", argv[argc]);
        size_t arg_len = strlen(argv[argc]) + 1; 

        if (arg_len > MAX_ARG_LEN){
            printf("ARG LONGER THAN ALLOWED!\n");
            return -1;            
        }
        sp -= arg_len;
        sp -= sp % 16; // riscv sp must be 16-byte aligned

        if(sp < proc->user_sp_start){
            printf("STACK OVERFLOW!!\n");
            return -2;
        }

        memcpy( (void *) sp, (void *) argv[argc], arg_len);
        argv_pointers[argc] = VADDR_USER_STACK_HARD_END- (paddr_sp_end- sp);
        debug_printf("Copied arg to 0x%x, vaddr: 0x%x", sp, argv_pointers[argc]);
        debug_printf(" value: '%s'\n", sp);
    }
    argv_pointers[argc] = 0;

    // Finally push to the stack.. the actually array of pointer i.e argv_pointers
    size_t argv_bytes_size = (argc+1) * sizeof(paddr_t); 
    sp -= argv_bytes_size; // +1 for the extra 0
    sp -= sp % 16;

    if(sp < proc->user_sp_start){
        printf("STACK OVERFLOW!!\n");
        return -2;
    }

    memcpy((void *) sp, (void *) &argv_pointers[0], argv_bytes_size);
    
    *sp_out = sp;
    return argc;
}


extern char __free_ram[], __free_ram_end[];
void init_process_pde(struct Proc * proc){
    //
    // ALLOC OF Page directory table! .. 1024 entries of 32bits, that are the configs of page tables.. allocated dynamically
    //
    
    proc->pde_paddr = (paddr_t)init_user_pde_table();
    uint32_t *pde_table = (uint32_t *) proc->pde_paddr;
    
    debug_printf("FOR PROC %u MAP PAGETABLE %x\n", proc->pid, proc->pde_paddr);

    // First map page for page table as direct map
    map_page(pde_table, proc->pde_paddr, proc->pde_paddr, KERNEL_PERMISSIONS_ALL);
    
    //
    // ALLOC OF Process stack
    //

    // Map the code of the kernel so that when a syscall/trap happens there is no page fault. No permission for user. Direct map.
    debug_printf("FOR PROC %u MAP KERNEL CODE %x to %x\n", proc->pid, get_paddr_kernel_start(), get_paddr_kernel_end());
    direct_map_range(pde_table, 
            get_paddr_kernel_start(),
            get_paddr_kernel_end(),
            KERNEL_PERMISSIONS_ALL
    );

    // Mem layout is Direct mapping for users? For easier management for now.
    // First map page for page table as direct map
    // Map user stack to vaddr
    // [USER_PROG_HARD_END ..USER_STACK_PAGE_COUNT .. USER_STACJ_HARD_END] ..    
    // ... to avoid having to calculate dynamic start. Also ... no page for safeguard yet. 
    //

    // paddr_t paddr_sp_end = proc->user_sp_start + USER_STACK_PAGE_COUNT * PAGE_SIZE;
    // offset_map_range(pde_table, proc->user_sp_start, paddr_sp_end,
    //         USER_PERMISSIONS_ALL, VADDR_USER_HARD_END); 

    // direct_map_range(pde_table, // SHOULD NOT BE DONE!!!!!!!!!!!!
    //     (paddr_t)__free_ram,
    //     (paddr_t)__free_ram_end,
    //     KERNEL_PERMISSIONS_ALL
    // );    
}


void load_create_process_user(struct Proc * proc, const struct AppBinaryInfo * app_info, char ** argv) {    
    init_process_pde(proc);

    proc->pc = VADDR_USER_BASE; // Entry point is setted to the vaddr, here it could be the trampoline but for now is the code of prog   

    // Map the code of the user program/binary... loading it from memory
    vaddr_t curr_vaddr = VADDR_USER_BASE;
    for (uint32_t off = 0; off < app_info->size; off += PAGE_SIZE) {
        paddr_t curr_page_paddr = alloc_pages(1); // Alloc pages throws PANIC ALREADY!

        // paddr_t curr_page_paddr;
        // if(try_alloc_user_page(&curr_page_paddr) < 0){
        //     PANIC("Failed alloc user page in load code for process!\n");
        //     // debug_printf("Failed alloc user page in load code for process!\n");
        //     return;
        // }
        
        if(off == 0){
            debug_printf("PADDR START OF PROCESS 0x%x\n",curr_page_paddr);
        }

        // Handle the case where the data to be copied is smaller than the page size.
        size_t remaining = app_info->size - off;
        size_t copy_size = (PAGE_SIZE <= remaining) ? PAGE_SIZE : remaining;

        // Copiar los datos a la página física recién asignada
        // A futuro... no muy lejano.... esto no seria con memcpy, sino accediendo a disco, asi no se carga a memoria todos los programas.
        memcpy((void *) curr_page_paddr, app_info->start + off, copy_size);

        // Map the loaded code to the VADDR of the user programs
        map_page((uint32_t*) proc->pde_paddr, curr_vaddr, curr_page_paddr,
                 USER_PERMISSIONS_ALL);
        
        curr_vaddr+= PAGE_SIZE;
    }

    // curr_vaddr is the first page not to be used by the process! i.e if it is 0x160000 then this could be the stack start.. for now ignored.
    printf("Process %u uses up to vaddr: 0x%x\n",proc->pid, curr_vaddr);
    

    init_proc_pages(proc);

    proc->status = PROC_RUNNABLE;

    //
    // ARGV/init parameters passing
    //

    paddr_t final_user_sp_top;
    int argc= set_init_parameters_for_proc(proc, argv, &final_user_sp_top);
    if (argc < 0){
        PANIC("ERROR When allocating params for proc!");
    }
    
    paddr_t paddr_sp_end = proc->user_sp_start + USER_STACK_PAGE_COUNT * PAGE_SIZE;
    paddr_t params_total_len = paddr_sp_end - final_user_sp_top; // How many bytes does this ocuppy
    // Sets on the trapframe th pc to the right vl

    vaddr_t params_vaddr = VADDR_USER_STACK_HARD_END - params_total_len;
    debug_printf("Proc has sp top 0x%x after params at: 0x%x, len: %u so vaddr 0x%x\n", paddr_sp_end, final_user_sp_top, params_total_len, params_vaddr);

    // Sets sp to the virtual stack end - len of params.. params start vaddr, so that it does not use it for the proc
    init_trapframe(proc, params_vaddr); 
    
    struct TrapFrame * proc_tf = &proc->tf;

    SET_SYSCALL_RET0(proc_tf, argc);
    SET_SYSCALL_RET1(proc_tf, params_vaddr);

}

// For now no extra mapping needed.
void load_create_process_kernel(struct Proc * proc, uint32_t proc_entry){
    PANIC("Not implemented kernel process yet");
} 
