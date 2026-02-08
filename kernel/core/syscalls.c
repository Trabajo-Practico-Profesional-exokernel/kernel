 
#include "arch/trap_handling.h"
#include "arch/logging.h"
#include "string.h"
#include "stdlib.h"
#include "arch/stdio.h"
#include "arch/trap.h"
#include "sched.h"
#include "stdio.h"
#include "console/debug.h"
#include "arch/console.h"
#include "console_files.h"
#include "proc_fs.h"

void syscall_console_put(FullTrapFrame *tf, uintptr_t pc) {

    uint32_t buf_vaddr = SYSCALL_ARG0(tf);
    uint32_t len = SYSCALL_ARG1(tf);
    struct Proc *current_proc = myproc();

    switch_to_kernel_tables();

    uint32_t buf_paddr = get_paddr_for((uint32_t*)current_proc->pde_paddr, buf_vaddr);

    int bytes_write = console_file_write(current_proc->pid, STDOUT, (int*)buf_paddr, len);

    SET_SYSCALL_RET0(tf, bytes_write);

    switch_page_table((uint32_t *) current_proc->pde_paddr);

}


void syscall_console_get(FullTrapFrame *tf, uintptr_t pc) {

    uint32_t buf_vaddr = SYSCALL_ARG0(tf);
    uint32_t len = SYSCALL_ARG1(tf);
    struct Proc *receiver_proc = myproc();

    switch_to_kernel_tables();

    uint32_t buf_paddr = get_paddr_for((uint32_t*)receiver_proc->pde_paddr, buf_vaddr);

    int bytes_read = console_file_read(receiver_proc->pid, STDIN, (int*)buf_paddr, len);

    if (bytes_read <= 0 ) { // Means no message available, so block
        receiver_proc->status = PROC_NOT_RUNNABLE;
        console_add_waiter(receiver_proc->pid);

        save_curr_proc_state(tf, pc);
        sched_yield();
    }

    SET_SYSCALL_RET0(tf, bytes_read);

    switch_page_table((uint32_t *) receiver_proc->pde_paddr);

}


void syscall_console_close(FullTrapFrame *tf, uintptr_t pc){
    uint32_t fd = SYSCALL_ARG0(tf);
    uint32_t pid = myproc()->pid;

    int res = console_file_close(pid, fd);

    SET_SYSCALL_RET0(tf, res);
}


void syscall_proc_ls(FullTrapFrame *tf, uintptr_t pc){
    proc_ls();
    SET_SYSCALL_RET0(tf, SUCCESS);
}



#define MAX_SYSCALLS 50
syscall_handler_t syscall_table[MAX_SYSCALLS] = {
    [SYS_CONSOLE_PUT] = syscall_console_put,
    [SYS_CONSOLE_GET] = syscall_console_get,
    [SYS_CONSOLE_CLOSE] = syscall_console_close,
    [SYS_PROC_LS] = syscall_proc_ls,
    // ... other handlers, wil be registered with register_syscall
};
void register_syscall(size_t sysno, syscall_handler_t handler){
    syscall_table[sysno] = handler;
}

uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc) {
    unsigned sysno = SYSCALL_SYSNO(tf);

    if (sysno >= MAX_SYSCALLS || syscall_table[sysno] == NULL) {
        debug_printf("unexpected syscall a3=%x max sysno: %x at pc: %x\n", sysno, MAX_SYSCALLS, pc);
        printTrapFull(tf);
    } else {
        // #if IS_RISC
        // #else
        // debug_printf("Asked to exec syscall sysno=%u max sysno: %x at pc: %x\n", sysno, MAX_SYSCALLS, pc);
        // #endif
        syscall_table[sysno](tf, pc);
    }

    return pc + 4; // skip ecall
}
