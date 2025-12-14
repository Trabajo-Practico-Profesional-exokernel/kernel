#include "arch/trap_handling.h"
#include "inc/common.h"

// Sched exec , wait and so on...
#include "sched.h"
#include "proc.h"
#include "arch/mem.h" // needed for switch to kernel page tables
#include "arch_inc/trap_constants.h"
#include "arch/logging.h"
#include "std/string.h"

#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  
#ifdef IS_RISC
// meta/gen/apps_meta.c defines this...
extern struct AppBinaryInfo _binary_apps[];
#else
struct AppBinaryInfo _binary_apps[10];
#endif



void syscall_exec(FullTrapFrame *tf, uintptr_t pc) {

    int prog_ind = SYSCALL_ARG0(tf);

    if (prog_ind < 0 || prog_ind>= APP_COUNT){
        printf("Invalid exec call ind %d \n", prog_ind);
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)        
        return;
    }
    printf("Should run program at ind %d \n", prog_ind);
    
    struct Proc* proc= get_first_free_proc();
    // It cannot but NULL it throws panic for now but check it anyway for the future!
    if (proc == NULL){
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)
        return;
    }

    printf("Should run free proc %p binary: %p \n", proc, &_binary_apps[prog_ind]);

    #ifdef IS_RISC
    switch_to_kernel_tables();
    load_create_process_user(proc, &_binary_apps[prog_ind]);
    
    // Do switch to new proc? ... no?
    SET_SYSCALL_RET0(tf, proc->pid)

    // Switch back to page table of user!
    switch_page_table((uint32_t *)(get_curr()->pde_paddr));
    #else

    load_create_process_user(proc, &_binary_apps[prog_ind]);
    // Now do switch? or not? naaa If you want you could wait for it! after ret.
    SET_SYSCALL_RET0(tf, proc->pid)
    #endif

}


void syscall_exit(FullTrapFrame *tf, uintptr_t pc){
    int exit_code = SYSCALL_ARG0(tf);
    struct Proc * exited_proc = get_curr();
    printf("Process %u exited with code %d\n", exited_proc->pid, exit_code);

    #ifdef IS_RISC
    switch_to_kernel_tables();
    #endif
    
    // Delete! .. from proc.c
    free_process(exited_proc);

    // TO DO! Notify for processes waiting?
    sched_yield();
}


void syscall_wait(FullTrapFrame *tf, uintptr_t pc){
    int waited_proc = SYSCALL_ARG0(tf);
    struct Proc * waiting_proc = get_curr();
    printf("Process %d should wait blocked for %d exit!(For now just yield!)\n",waiting_proc->pid,  waited_proc);
    
    SET_SYSCALL_RET0(tf, 0) // On tf saved... save hardcoded ret code for waited proc

    save_curr_proc_state(tf, pc);
    sched_yield();    
}


void syscall_yield(FullTrapFrame *tf, uintptr_t pc){
    save_curr_proc_state(tf, pc);
    sched_yield();
}


void init_syscalls_proc(void) {
    register_syscall(SYS_EXEC, syscall_exec);
    register_syscall(SYS_EXIT, syscall_exit);
    register_syscall(SYS_WAIT, syscall_wait);
    register_syscall(SYS_YIELD, syscall_yield);
}