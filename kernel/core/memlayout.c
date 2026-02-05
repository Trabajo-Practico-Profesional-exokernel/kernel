#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem_layout.h"
#include "string.h"
#include "stdlib.h"
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.
#include "constants.h"
#include "stdio.h"
#include "console/debug.h"
#include "user_pages_alloc.h"
#include "proc_sleeping.h"
#include "proc_fs.h"
#include "proc.h"
#include "sched.h"
#include "proc_pages.h"
#include "proc_syscalls.h"
#include "arch/logging.h"

// #include "user_pages_alloc.h"
extern char __idle_proc_start[], __idle_proc_end[];

void init_idle_proc(void){
    struct Proc * sched_idle_proc = get_idle_proc();
    sched_idle_proc->pid = PROCS_MAX;
    init_process_pde(sched_idle_proc);

    sched_idle_proc->pc = VADDR_USER_BASE;
    map_page((uint32_t*) sched_idle_proc->pde_paddr, VADDR_USER_BASE, (paddr_t) __idle_proc_start,
             USER_PERMISSIONS_ALL); 
    
    init_proc_stack(sched_idle_proc);
    init_trapframe(sched_idle_proc, VADDR_USER_STACK_HARD_END); 
}



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

    init_proc_uptime(proc);
    add_proc_info(proc);
    int actual_memory = get_free_user_memory();
    update_system_memory(actual_memory);
}

// For now no extra mapping needed.
void load_create_process_kernel(struct Proc * proc, uint32_t proc_entry){
    PANIC("Not implemented kernel process yet");
} 



int load_create_forked(struct Proc* parent, struct Proc* child){
    init_process_pde(child);

    int ret = copy_mem_pages(parent, child);

    if(ret < 0){
        free_process(child);
        return ret;
    }

    child->tf = parent->tf;
    child->pc = parent->pc;

    struct TrapFrame * child_tf = &child->tf;
    SET_SYSCALL_RET0(child_tf, 0); // Set to 0 so that is flagged to be a child!

    child->status = PROC_RUNNABLE;
    init_proc_uptime(child);

    return 0;
}




