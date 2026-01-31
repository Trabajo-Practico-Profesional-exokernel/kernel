#ifndef INC_PROC
#define INC_PROC
#include "types.h"
#include "arch_inc/trapframe.h"
#include "arch_inc/mem_constants.h"

// Forward declaration of MessageQueue if not already defined

#include "mem.h"

// Include constants defining positions in memory, for the utility of whom includes proc.h
#include "arch/mem_layout.h"

#define LOG2NPROC 4
#define NPROC (1 << LOG2NPROC)

#define PROCS_MAX NPROC//8       // Maximum number of processes

#define PROCX(procid) ((procid) & (PROCS_MAX - 1))

// Just 256 bytes .. so that 8* 256 = 1KB + some bytes for pointers .. args can be passed through 1 page of 4096.. in the user stack..
#define MAXARG 8
#define MAX_ARG_LEN 256 

#define MAX_FILES 16

typedef int32_t procid_t;

//#define PROC_UNUSED   0   // Unused process control structure
// Values of status in struct Proc
enum { PROC_FREE = 0, PROC_DYING, PROC_RUNNABLE, PROC_RUNNING, PROC_NOT_RUNNABLE };


struct Proc {
    struct TrapFrame tf;
    uintptr_t pc; // process current ins
    paddr_t pde_paddr;      // Physical address for Page Directory: C3 for x86 or SATP for RISCV

    // Pointer to the physical address of the user stack start... i.e min paddr
    // user stack goes from [user_sp_start ..USER_STACK_PAGE_COUNT .. initial_user_stack_top] .. user stack grows up to sp_start.
    paddr_t user_sp_start; 

    procid_t pid;             // Process ID

    int gid;                // Group ID

    int status;           // Process state: PROC_FREE or PROC_RUNNABLE,  PROC_DYING, PROC_RUNNABLE, PROC_RUNNING, PROC_NOT_RUNNABLE 

    int cpunum; // The CPU that the env is running on

};

// struct ProcMessageQueue*
// struct ProcMessageQueue

#endif /* !*/
