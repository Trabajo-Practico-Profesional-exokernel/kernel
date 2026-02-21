#ifndef PROC_SYSCALLS_H
#define PROC_SYSCALLS_H
#include "types.h"
#include "arch/proc.h"
#include "arch_inc/trapframe.h"

struct ProcExitStatus {
    procid_t proc_pid;
    int ret_code;
    struct ProcExitStatus* waiters_head;
    struct ProcExitStatus* waiters_tail;
    
    struct ProcExitStatus* link_next_waiting;
};
void reset_exit_status(struct ProcExitStatus* status);
struct ProcExitStatus* get_exit_status(int pid);

int set_init_parameters_for_proc(struct Proc * proc, char ** argv, paddr_t* sp_out);

int copy_argv_pointers_from_user(struct Proc * proc, paddr_t* argv_pointers, vaddr_t vaddr_argv);
void init_syscalls_ipc(void);



void syscall_exec(FullTrapFrame *tf, uintptr_t pc);
void syscall_execv(FullTrapFrame *tf, uintptr_t pc);
void syscall_fork(FullTrapFrame *tf, uintptr_t pc);

void syscall_try_send_content(FullTrapFrame *tf, uintptr_t pc);
void syscall_try_recv_content(FullTrapFrame *tf, uintptr_t pc);
void syscall_recv_content(FullTrapFrame *tf, uintptr_t pc);

#endif /* !*/
