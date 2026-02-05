#include "proc_sleeping.h"
#include "sched.h"
#include "arch/clock_checks.h"
#include "console/debug.h"
#include "proc_fs.h"

volatile uint64_t ticks = 0;


struct ProcTimingInfo {
    uint32_t uptime_ticks;
    uint64_t locked_until_ticks;
    bool free; // If free its ignored.
    // struct ProcTimingInfo * next_sleeping; // For future optimizations?
};

struct ProcTimingInfo procs_timing_info[PROCS_MAX + 1];
// +1 == added one extra for idle proc to count how many ticks were idle! 
// idle proc pid == PROCS_MAX 

void reset_proc_uptime(struct Proc*  proc){
    struct ProcTimingInfo* info = &procs_timing_info[proc->pid];
    info->uptime_ticks = 0;
    info->locked_until_ticks = 0;
    info->free = true;
}

void init_proc_uptime(struct Proc*  proc){
    procs_timing_info[proc->pid].free = false;
}

uint32_t get_total_active_ticks(void) {
    uint32_t total = 0;
    for (int i = 0; i < PROCS_MAX; i++) {
        if (!procs_timing_info[i].free) {
            total += procs_timing_info[i].uptime_ticks;
        }
    }
    return total;
}


void check_sleeping_proc(void){
    ticks+=1; // Only on this core! should increment/check on main core!
    int real_ticks = get_total_active_ticks();

    update_system_info(ticks, real_ticks);
    struct Proc * curr_proc = get_curr(); 

    if(curr_proc->status == PROC_RUNNING){
        struct ProcTimingInfo* curr_info = &procs_timing_info[curr_proc->pid];
        curr_info->uptime_ticks+=1;
    }

    int pid;
    for (pid = 0; pid < PROCS_MAX; pid++) {
        if (procs_timing_info[pid].free) {
            continue;
        }

        if (procs_timing_info[pid].locked_until_ticks == ticks){
            struct Proc * sleeping_proc= get_proc(pid);
            sleeping_proc->status = PROC_RUNNABLE;
            // printf("---> AWAKENED PROC %d status is runnable? %u \n", pid, 
            //     sleeping_proc->status == PROC_RUNNABLE? 1: 0);
        } 
    }
}







void syscall_sleep(FullTrapFrame *tf, uintptr_t pc){
    uint32_t ticks_to_sleep = SYSCALL_ARG0(tf);
    struct Proc * curr_proc = get_curr();
    struct ProcTimingInfo* info = &procs_timing_info[curr_proc->pid];
    info->locked_until_ticks = ticks + ticks_to_sleep;
    // printf("[Sleep] proc %d, should awake at %u now at: %u\n", curr_proc->pid, info->locked_until_ticks,ticks);
    curr_proc->status = PROC_NOT_RUNNABLE;
    
    save_curr_proc_state(tf, pc+ 4);
    sched_yield();
}

void syscall_uptime(FullTrapFrame *tf, uintptr_t pc){
    struct Proc * curr_proc = get_curr();
    struct ProcTimingInfo* info = &procs_timing_info[curr_proc->pid];

    SET_SYSCALL_RET0(tf, info->uptime_ticks);
}


uint64_t get_real_ticks(void){
    return ticks;
}
uint32_t get_idle_ticks(void){
    struct ProcTimingInfo* curr_info = &procs_timing_info[PROCS_MAX];
    return curr_info->uptime_ticks;
}
