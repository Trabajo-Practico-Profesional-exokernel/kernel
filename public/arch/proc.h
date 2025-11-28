#ifndef INC_PROC
#define INC_PROC
#include "inc/types.h"
#include "arch_inc/trapframe.h"
#include "arch_inc/mem_constants.h"
#include "communication.h"
#include "mem.h"

#define LOG2NPROC 10
#define NPROC (1 << LOG2NPROC)

#define PROCS_MAX NPROC//8       // Maximum number of processes

#define PROCX(procid) ((procid) & (PROCS_MAX - 1))


#define KERN_STACK_PAGES 2


typedef int32_t procid_t;

//#define PROC_UNUSED   0   // Unused process control structure
// Values of status in struct Proc
enum { PROC_FREE = 0, PROC_DYING, PROC_RUNNABLE, PROC_RUNNING, PROC_NOT_RUNNABLE };

struct Proc {
    struct TrapFrame tf;
    uintptr_t pc; // process current ins
    paddr_t pde_paddr;      // Physical address for Page Directory: C3 for x86 or SATP for RISCV
    vaddr_t kernel_sp;          // Stack pointer

    procid_t pid;             // Process ID
    int status;           // Process state: PROC_FREE or PROC_RUNNABLE,  PROC_DYING, PROC_RUNNABLE, PROC_RUNNING, PROC_NOT_RUNNABLE 

    int cpunum; // The CPU that the env is running on

    struct ProcMessageQueue proc_msg_queue;
};

#endif /* !*/
