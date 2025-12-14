#ifndef PROC_SYSCALLS_H
#define PROC_SYSCALLS_H
#include "inc/types.h"
#include "arch/proc.h"

struct ProcExitStatus {
    procid_t proc_pid;
    int ret_code;
    struct ProcExitStatus* waiters_head;
    struct ProcExitStatus* waiters_tail;
    
    struct ProcExitStatus* link_next_waiting;
};

#endif /* !*/
