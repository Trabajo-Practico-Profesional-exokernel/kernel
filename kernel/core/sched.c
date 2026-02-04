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

extern void sched_finish(bool curr_is_blocked);

#define MAX_TIME_SLICES 45


uint32_t curr_slices = 0;
int filesystem_PID = -1;
int coordinator_PID = -1;

struct Proc *curr;

struct Proc * get_curr(){
    return curr;
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

#define NUM_CPUS 4
#define TRAMPOLINE_STACK_SIZE 4096 // 1 page essentially?
uint8_t trampoline_stacks[NUM_CPUS][TRAMPOLINE_STACK_SIZE]; // All process control structures.


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


void switch_to_idle_proc(void){
    curr = &idle_proc;
    curr->status = PROC_RUNNING;
    curr_slices = MAX_TIME_SLICES;

    // printf("--------------------------- SWITCH IDLE PROC %u \n", curr->pid);
    #ifdef IS_RISC
    SSCRATCH_NEW_STACK(&trampoline_stacks[curr->cpunum][TRAMPOLINE_STACK_SIZE])
    #endif
    switch_page_table((uint32_t *)curr->pde_paddr);
    switch_context(curr);
}

void switch_proc(struct Proc* next) {
    curr = next;
    if(curr != &idle_proc){
        curr_slices = 0; // Reset clock slices for new proc.
    }
    curr->status = PROC_RUNNING;

    // debug_printf("[NEXT PROC BEFORE SWITCH_CONTEXT] ");
    // printProc(next);
    // debug_printf("\n\n");
    // debug_printf("[DEBUG] Target EIP: %x | Target ESP: %x\n", curr->tf.eip, curr->tf.esp);

    debug_printf("--------------------------- SWITCH TO PROC %u \n", curr->pid);

    #ifdef IS_RISC
    // Now we are not using kernel stack pointers of process at this point... so no need to switch stack
    // SWITCH_TO_STACK(&trampoline_stacks[curr->cpunum][TRAMPOLINE_STACK_SIZE])
    // SSCRATCH_STACK() // Save for next trap to use this stack pointer i.e trampoline
    // debug_printf("----> trampoline sscratch stack top %p \n", &trampoline_stacks[curr->cpunum][TRAMPOLINE_STACK_SIZE]);
    // BUUT you have to sscratch it for next trap since its not being restored like the end of trapentry would.
    
    SSCRATCH_NEW_STACK(&trampoline_stacks[curr->cpunum][TRAMPOLINE_STACK_SIZE])
    #endif
    
    switch_page_table((uint32_t *)curr->pde_paddr);

    switch_context(curr);
}

// Cambia la firma y el cuerpo:
void clock_yield(FullTrapFrame *tf, uintptr_t proc_pc) {  // Quitamos uintptr_t proc_pc

    curr_slices += 1;
    if(curr){ // Only If there is a valid process running even If blocked.
        check_sleeping_proc();

        if(curr == &idle_proc && curr_slices % MAX_TIME_SLICES == 0){
            debug_printf("[TICK] idle time slice tot idle: %u tot ticks = %u\n",get_idle_ticks(), get_real_ticks());
        }

        // NOT implemented yet
        //check_io_locked_proc();
        //check_stdin_locked_proc();
        //check_ipc_locked_proc();
    }

    if (curr != &idle_proc && curr_slices < MAX_TIME_SLICES){
        return;
    }

    curr->pc = proc_pc; 
    update_trapframe(curr, tf);


    sched_yield();
}

void save_curr_proc_state(FullTrapFrame *tf, uintptr_t proc_pc){
    curr->pc = proc_pc;
    update_trapframe(curr, tf);    
}


void sched_yield(void) {
    // #ifdef IS_RISC
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
    bool curr_is_blocked = false;

    if(curr){
        if(curr->status == PROC_RUNNABLE) {
            // keep runing the last proc while it exists
            switch_proc(curr);
        } else {
            curr_is_blocked = true;
            curr = NULL;            
        }
    }

    sched_finish(curr_is_blocked);
}
