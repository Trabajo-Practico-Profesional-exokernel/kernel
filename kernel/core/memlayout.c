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
#include "console_files.h"

// #include "user_pages_alloc.h"
#define IDLE_PROC_STACK_SIZE 4096 // 1 page essentially?

char idle_proc_stack[IDLE_PROC_STACK_SIZE];


void init_idle_proc(void){
    struct Proc * sched_idle_proc = get_idle_proc();
    sched_idle_proc->pid = PROCS_MAX;
    sched_idle_proc->pde_paddr = (paddr_t) get_kernel_pde();
    sched_idle_proc->pc = (vaddr_t) &idle_main;
    init_trapframe(sched_idle_proc, (vaddr_t) &idle_proc_stack[IDLE_PROC_STACK_SIZE]); 
}



void init_process_pde(struct Proc * proc){
    //
    // ALLOC OF Page directory table! .. 1024 entries of 32bits, that are the configs of page tables.. allocated dynamically
    //
    
    proc->pde_paddr = (paddr_t)init_user_pde_table();
    uint32_t *pde_table = (uint32_t *) proc->pde_paddr;
    
    VERBOSE_DEBUG_PRINTF("FOR PROC %u MAP PAGETABLE %x\n", proc->pid, proc->pde_paddr);

    // First map page for page table as direct map
    map_page(pde_table, proc->pde_paddr, proc->pde_paddr, KERNEL_PERMISSIONS_ALL);
    
    //
    // ALLOC OF Process stack
    //

    // Map the code of the kernel so that when a syscall/trap happens there is no page fault. No permission for user. Direct map.
    VERBOSE_DEBUG_PRINTF("FOR PROC %u MAP KERNEL CODE %x to %x\n", proc->pid, get_paddr_kernel_start(), get_paddr_kernel_end());
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


int init_parameters_for_proc(struct Proc * proc, char ** argv, char ** envp){
    //
    // ARGV and ENVP parameters passing
    //
    VERBOSE_PRINTF("Init parameters for proc %s argv:%p envp: %p\n",proc->proc_name, argv, envp);

    paddr_t paddr_sp_end = proc->user_sp_start + USER_STACK_PAGE_COUNT * PAGE_SIZE;
    paddr_t final_user_sp_top = paddr_sp_end;
    paddr_t param_pointers[MAXARG];

    int argc = copy_to_stack_list(&param_pointers[0],argv, 
                &final_user_sp_top, proc->user_sp_start, VADDR_USER_STACK_HARD_END);

    if (argc < 0){
        VERBOSE_PRINTF("ERROR When allocating argv for proc!");
        return argc;
    }

    vaddr_t argv_vaddr = VADDR_USER_STACK_HARD_END - (paddr_sp_end - final_user_sp_top);

    int env_argc = copy_to_stack_list(&param_pointers[0],envp, 
                &final_user_sp_top, proc->user_sp_start, argv_vaddr);

    if (env_argc < 0){
        VERBOSE_PRINTF("ERROR When allocating envp for proc!");
        return argc;
    }
    proc->envp_list_start = final_user_sp_top;

    vaddr_t envp_vaddr = VADDR_USER_STACK_HARD_END - (paddr_sp_end - proc->envp_list_start);

    VERBOSE_PRINTF("Proc has sp top 0x%x after argv vaddr: 0x%x, envp vaddr: 0x%x\n", paddr_sp_end, 
            argv_vaddr, envp_vaddr);

    // Sets sp to the virtual stack end - len of params.. envp_vaddr start vaddr, so that it does not use it for the proc
    
    struct TrapFrame * proc_tf = &proc->tf;

    // Init sp, in x86 also sets ebp to this.
    INIT_TRAPFRAME_SP(proc_tf, envp_vaddr);

    SET_SYSCALL_RET0(proc_tf, argc);
    SET_SYSCALL_RET1(proc_tf, argv_vaddr);
    SET_SYSCALL_RET2(proc_tf, envp_vaddr);

    return 0;
}










int load_create_process_user(struct Proc * proc, const struct BinaryAppEntry * app_info) {    
    init_process_pde(proc);

    proc->pc = VADDR_USER_BASE; // Entry point is setted to the vaddr, here it could be the trampoline but for now is the code of prog   

    // Map the code of the user program/binary... loading it from memory
    vaddr_t curr_vaddr = VADDR_USER_BASE;
    
    int err = load_app_code_to_user_mem(app_info, &curr_vaddr, (uint32_t*) proc->pde_paddr);
    if(err < 0){
        VERBOSE_PRINTF("Failed to load app code to memory for proc %u err: %d\n", proc->pid, err);
        return err;
    }
    
    // curr_vaddr is the first page not to be used by the process! i.e if it is 0x160000 then this could be the stack start.. for now ignored.
    VERBOSE_PRINTF("Process %u uses up to vaddr: 0x%x\n",proc->pid, curr_vaddr);

    init_proc_pages(proc);

    init_trapframe(proc, VADDR_USER_STACK_HARD_END); // Reset values

    proc->status = PROC_RUNNABLE;

    // Uptime and more info
    strcpy(proc->proc_name, app_info->name);
    
    init_proc_uptime(proc);
    proc->gid = proc->pid;

}

// For now no extra mapping needed.
int load_create_process_kernel(struct Proc * proc, uint32_t proc_entry){
    PANIC("Not implemented kernel process yet");
} 


int reload_process_user(struct Proc * proc, const struct BinaryAppEntry * app_info) {
    vaddr_t curr_vaddr = VADDR_USER_BASE;
    
    int err = reload_app_code_to_user_mem(app_info, &curr_vaddr, (uint32_t*) proc->pde_paddr);
    if(err < 0){
        // Probably corrupted code segment. For now it panics.
        PANIC("Failed to reload app code to memory for proc %u err: %d\n", proc->pid, err);
    }    
    reset_proc_range(proc, curr_vaddr, VADDR_USER_HARD_END);
    
    proc->pc = VADDR_USER_BASE; // Entry point is setted to the vaddr, here it could be the trampoline but for now is the code of prog   
    init_trapframe(proc, VADDR_USER_STACK_HARD_END); // Reset values
    
    strcpy(proc->proc_name, app_info->name);
    return 0;
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
    
    strcpy(child->proc_name, parent->proc_name);
    init_proc_uptime(child);
    
    init_proc_std_files(child->pid);
    child->gid = parent->gid;

    return 0;
}



char *EMPTY_LIST[] = { 0 };


struct Proc * create_process_from_ind(int ind, char ** argv, char ** envp){
    struct Proc * proc = get_first_free_proc();
    init_proc_std_files(proc->pid);
    proc->gid = 0;

    struct BinaryAppEntry* app = get_app_from_ind(ind);
    if(app == NULL){
        PANIC("Invalid app ind %d to create process!", ind);
    }

    if(envp == NULL){
        envp = &EMPTY_LIST[0];
    }
    if(argv == NULL){
        argv = &EMPTY_LIST[0];
    }
    
    int err = load_create_process_user(proc, app);
    
    if (err >=0){
        err = init_parameters_for_proc(proc, argv, envp);
    }

    if(err < 0){
        PANIC("Failed create process from ind. %d\n", ind);
    }
    add_proc_to_system_stats(proc);

    return proc;
}

struct Proc * create_process(char* proc_name, char ** argv, char ** envp){
    int ind = get_app_from_name(proc_name);

    if(ind < 0){
        PANIC("Invalid app name %s to create process!", proc_name);
    }

    return create_process_from_ind(ind, argv, envp);
}


