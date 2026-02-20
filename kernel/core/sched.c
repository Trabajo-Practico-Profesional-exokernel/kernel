#include "sched.h"
#include "proc.h"
#include "arch/logging.h"
#include "constants.h"
//#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.
#include "stdio.h"
#include "stdio.h"
#include "console/debug.h"

#include "arch/mem_layout.h"
#include "arch/clock_checks.h"

#include "arch/console.h"
#include "arch/stdio.h"
#include "arch/clock_checks.h"
#include "arch/spin_locks.h"
#include "arch/cpus.h"

#include "arch_inc/trap_constants.h"
#include "arch_inc/idle.h"


extern void sched_finish(struct Proc * last_proc);
#ifdef IS_TESTING
extern void on_clock_yield(uint32_t new_curr_slices);
#endif

#define MAX_TIME_SLICES 45


struct spinlock lock_scheduler;


int filesystem_PID = -1;
int coordinator_PID = -1;

struct Proc * myproc(){
    return mycpu()->proc;
}


struct Proc procs[PROCS_MAX]; // All process control structures.

struct Proc idle_proc;

struct Proc * get_idle_proc(){
    return &idle_proc;
}

// Uneeded for!
struct Proc *get_proc_by_pid(int pid){
    for (int i=0; i<PROCS_MAX; i++){
        if (procs[i].pid == pid) {
            return &procs[pid];
        }
    }
    PANIC("Process does not exist");    
}

struct Proc * get_proc(procid_t proc_pid){
    return &procs[PROCX(proc_pid)];
}

#define TRAMPOLINE_STACK_SIZE 4096 // 1 page essentially?
uint8_t trampoline_stacks[NCPU][TRAMPOLINE_STACK_SIZE]; // All process control structures.


struct Proc * get_first_free_proc(){
    int i;
    for (i = 0; i < PROCS_MAX; i++) {
        if (procs[i].status == PROC_FREE) {
            procs[i].pid = i; // Set pid Now just in case.
            
            // Set as not runnable so that next get first doesnt get this one
            procs[i].status = PROC_NOT_RUNNABLE; 
            return &procs[i];
        }
    }

    // Or just rutn NULL... 
    PANIC("No free process slots at get first free proc");    
}


// Called from kernel for cpu when no process is runnable.
// It does the whole setup for a user proc. But it does not switch off 
void switch_to_idle_proc(void){
    acquire(&lock_scheduler);
    // printf("Switching to idle proc for cpu %d\n", cpuid());

    struct cpu* cpu = mycpu();
    if(cpu->proc != &idle_proc){
        cpu->proc = &idle_proc;
        idle_proc.status = PROC_NOT_RUNNABLE; // Just in case
        cpu->slices = 0;
        switch_page_table((uint32_t *)idle_proc.pde_paddr);
    }
    #ifdef IS_RISC
    SSCRATCH_NEW_STACK(&trampoline_stacks[cpuid()][TRAMPOLINE_STACK_SIZE])
    #endif
    release(&lock_scheduler);

    debug_printf("CPU %d entered IDLE\n", cpuid());
    enable_interrupts();
    enable_timer_interrupts();
    idle_main();
}
void idle_main(void){
    while(1){
        // Check/poll for unblock processes
        polling_checks();
        sched_yield();
    }    
}

void switch_proc(struct Proc* next) {
    struct cpu* cpu = mycpu();
    int cpunum = cpuid();
    cpu->proc = next;

    if(next != &idle_proc){
        cpu->slices = 0; // Reset clock slices for new proc.
    }
    next->status = PROC_RUNNING;
    next->cpunum = cpunum;

    release(&lock_scheduler);

    // printf("Switching to proc %d for cpu %d\n", next->pid, cpuid());

    #ifdef IS_RISC
    SSCRATCH_NEW_STACK(&trampoline_stacks[next->cpunum][TRAMPOLINE_STACK_SIZE])
    #endif
    
    switch_page_table((uint32_t *)next->pde_paddr);
    switch_context(next);
}

// Cambia la firma y el cuerpo:
void clock_yield(FullTrapFrame *tf, uintptr_t proc_pc) {  // Quitamos uintptr_t proc_pc

    struct cpu* curr_cpu = mycpu();
    curr_cpu->slices += 1;

    struct Proc* curr = curr_cpu->proc;
    
    #ifdef IS_TESTING
    on_clock_yield(curr_cpu->slices);
    #endif

    if(curr){ // Only If there is a valid process running even If blocked.
        check_sleeping_proc();

        if(curr == &idle_proc){
            add_idle_time(&idle_proc);

            if(curr_cpu->slices % MAX_TIME_SLICES == 0){
                debug_printf("[TICK] idle time slice tot idle: %u tot ticks = %u at cpu: %d \n",get_idle_ticks(), get_real_ticks(), cpuid());
            }
            return; // Go back.
        } else {
            add_uptime_to_proc(curr);
            // Check/poll for IO, Disk, etc. Drivers
            // polling_checks(); 
        }
    }

    if (curr_cpu->slices < MAX_TIME_SLICES){
        return;
    }

    curr->pc = proc_pc; 
    update_trapframe(curr, tf);

    sched_yield();
}

void save_curr_proc_state(FullTrapFrame *tf, uintptr_t proc_pc){
    struct Proc* curr = myproc();
    
    curr->pc = proc_pc;
    update_trapframe(curr, tf);    
}


void sched_yield(void) {
    acquire(&lock_scheduler);

    struct Proc * curr = myproc();

    ////
    //// Round robin!
    ////

    int currind = -1;

    if (curr){
        // If it was preemted, not in blocked state or so... then set it as runnable
        if (curr->status == PROC_RUNNING) {
            curr->status = PROC_RUNNABLE;  
        }

        currind= curr->pid;
    }

    int ind = currind + 1;

    // Look for next in range [curr+1 ; end] 
    while (ind < PROCS_MAX &&
           procs[ind].status !=
                   PROC_RUNNABLE) {
        ind++;
    }
    if (ind < PROCS_MAX) { 
        switch_proc(&procs[ind]);
    }

    // Now circular loop!
    // Look for next in range [0; curr] 
    ind = 0;

    while (ind < currind &&
           procs[ind].status !=
                   PROC_RUNNABLE) {  

        ind++;
    }


    if (ind < currind) {
        switch_proc(&procs[ind]);
    }

    // Found nothing , if curr is blocked or dead.. then reset it 

    if(curr){
        if(curr->status == PROC_RUNNABLE) {
            // keep runing the last proc while it exists
            switch_proc(curr);
        }
    }

    if(curr == &idle_proc){
        release(&lock_scheduler);
        return;    
    }

    struct Proc * last_proc = curr;

    // No current proc for cpu, wether it will be setted to idle or not, depends on sched_finish    
    mycpu()->proc = NULL; 

    release(&lock_scheduler);
    sched_finish(last_proc);
}
