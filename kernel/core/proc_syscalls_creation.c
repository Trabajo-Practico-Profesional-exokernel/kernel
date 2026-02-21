#include "arch/trap_handling.h"
#include "constants.h"

// Sched exec , wait and so on...
#include "sched.h"
#include "proc_syscalls.h"
#include "proc.h"
#include "arch/mem.h" // needed for switch to kernel page tables
#include "arch_inc/trap_constants.h"
#include "arch/logging.h"
#include "string.h"
#include "stdlib.h"

#include "stdio.h"
#include "console/debug.h"
#include "arch/ipc.h"
#include "console_files.h"

//extern struct AppBinaryInfo _binary_apps[];
#include "proc_disk_loading.h"


void syscall_execv(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t vaddr_proc_name = SYSCALL_ARG0(tf);
    vaddr_t vaddr_argv_pointer = SYSCALL_ARG1(tf);
    struct Proc* caller_proc = myproc();
    switch_to_kernel_tables();

    char * trg_proc_name  = (char *) get_paddr_for((uint32_t *) caller_proc->pde_paddr, vaddr_proc_name);
    if (trg_proc_name == 0){
        SET_SYSCALL_RET0(tf, -1)
        switch_page_table((uint32_t *) caller_proc->pde_paddr);
        return;        
    }
    
    paddr_t argv_pointers[MAXARG]; 
    
    if (vaddr_argv_pointer != 0){
        int argc = copy_argv_pointers_from_user(caller_proc, &argv_pointers[0], vaddr_argv_pointer);

        if (argc< 0){
            SET_SYSCALL_RET0(tf, argc)
            switch_page_table((uint32_t *) caller_proc->pde_paddr);
            return;
        }
    } else {
        VERBOSE_DEBUG_PRINTF("NO proc params execv\n");
        argv_pointers[0] = 0;
    }
    VERBOSE_DEBUG_PRINTF("EXECV should replace program with name '%s' for '%s' \n", caller_proc->proc_name, trg_proc_name);


    int prog_ind = get_app_from_name(trg_proc_name);
    struct BinaryAppEntry* app = NULL;

    if(prog_ind >= 0){
        app = get_app_from_ind(prog_ind);
    }

    if(app == NULL){
        VERBOSE_PRINTF("Invalid app ind %d to exec process from '%s'!\n", prog_ind,
            (char *) argv_pointers[0]);

        SET_SYSCALL_RET0(tf, -1);
        switch_page_table((uint32_t *) caller_proc->pde_paddr);
        return;
    }

    int ret = reload_process_user(caller_proc, app, (char **) &argv_pointers[0]);

    if(ret != 0){
        SET_SYSCALL_RET0(tf, ret);
        switch_page_table((uint32_t *) caller_proc->pde_paddr);
        return;
    }

    // Do sched yield to be fair?
    sched_yield();
    
}

void syscall_exec(FullTrapFrame *tf, uintptr_t pc) {

    vaddr_t vaddr_argv_pointer = SYSCALL_ARG0(tf);
    struct Proc* parent_proc = myproc();
    switch_to_kernel_tables();
    
    paddr_t argv_pointers[MAXARG]; 
    
    if (vaddr_argv_pointer != 0){
        int argc = copy_argv_pointers_from_user(parent_proc, &argv_pointers[0], vaddr_argv_pointer);

        if (argc< 0){
            SET_SYSCALL_RET0(tf, argc)
            switch_page_table((uint32_t *) parent_proc->pde_paddr);
            return;
        }
    } else {
        VERBOSE_PRINTF("Err: NO proc params exec\n");
        SET_SYSCALL_RET0(tf, -1)
        switch_page_table((uint32_t *) parent_proc->pde_paddr);
        return;
    }
    
    VERBOSE_DEBUG_PRINTF("EXEC should run program with name '%s' \n", (char *) argv_pointers[0]);

    int prog_ind = get_app_from_name((char *) argv_pointers[0]);
    struct BinaryAppEntry* app = NULL;

    if(prog_ind >= 0){
        app = get_app_from_ind(prog_ind);
    }

    if(app == NULL){
        VERBOSE_PRINTF("Invalid app ind %d to exec process from '%s'!\n", prog_ind,
            (char *) argv_pointers[0]);

        SET_SYSCALL_RET0(tf, -1);
        switch_page_table((uint32_t *) parent_proc->pde_paddr);
        return;
    }
    struct Proc* proc= get_first_free_proc();

    // It cannot but NULL it throws panic for now but check it anyway for the future!
    if (proc == NULL){
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)
        
        switch_page_table((uint32_t *) parent_proc->pde_paddr);
        return;
    }

    VERBOSE_DEBUG_PRINTF("Should run free proc %p binary: %s at off: %u \n", proc, app->name, app->start);

    load_create_process_user(proc, app, (char **) &argv_pointers[0]);

    VERBOSE_DEBUG_PRINTF("Loaded proc '%s'\n", app->name);
    reset_exit_status(get_exit_status(proc->pid));
    init_proc_std_files(proc->pid);

    // Do switch to new proc? ... no?
    SET_SYSCALL_RET0(tf, proc->pid)

    switch_page_table((uint32_t *) parent_proc->pde_paddr);
}




void syscall_fork(FullTrapFrame *tf, uintptr_t pc) {

    struct Proc* parent_proc = myproc();
    switch_to_kernel_tables();
    
    VERBOSE_DEBUG_PRINTF("Should fork program at ind %d \n", parent_proc->pid);
    
    struct Proc* child_proc= get_first_free_proc();

    if (child_proc == NULL){
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)
        save_curr_proc_state(tf, pc + 4);    
        
        switch_page_table((uint32_t *) parent_proc->pde_paddr);
        return;
    }
    uintptr_t trg_init_pc = pc + 4;
    
    SET_SYSCALL_RET0(tf, child_proc->pid)
    save_curr_proc_state(tf, trg_init_pc);

    int ret = load_create_forked(parent_proc, child_proc);

    if(ret < 0){
        SET_SYSCALL_RET0(tf, ret);
        save_curr_proc_state(tf, trg_init_pc);
    } else {
        reset_exit_status(get_exit_status(child_proc->pid));
    }


    switch_page_table((uint32_t *) parent_proc->pde_paddr);
}
