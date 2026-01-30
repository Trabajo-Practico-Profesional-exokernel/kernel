#ifndef PROC_SYSCALLS_H
#define PROC_SYSCALLS_H
#include "types.h"
#include "arch/proc.h"

struct ProcExitStatus {
    procid_t proc_pid;
    int ret_code;
    struct ProcExitStatus* waiters_head;
    struct ProcExitStatus* waiters_tail;
    
    struct ProcExitStatus* link_next_waiting;
};

int set_init_parameters_for_proc(struct Proc * proc, char ** argv, paddr_t* sp_out);

int copy_argv_pointers_from_user(struct Proc * proc, paddr_t* argv_pointers, vaddr_t vaddr_argv);
void init_syscalls_ipc(void);
#endif /* !*/
