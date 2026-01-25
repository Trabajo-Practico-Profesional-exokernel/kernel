#include "sched.h"
#include "proc.h"
#include "arch/logging.h"
#include "constants.h"
//#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

#include "arch/mem_layout.h"

#define MAX_TIME_SLICES 45

volatile uint64_t ticks = 0;

uint32_t curr_slices = 0;
int filesystem_PID = -1;

struct Proc *curr;

struct Proc * get_curr(){
    return curr;
}

struct Proc procs[PROCS_MAX]; // All process control structures.


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





void switch_proc(struct Proc* next) {
    curr = next;
    curr_slices = 0; // Reset clock slices for new proc.
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
    ticks+=1;
    
    if (curr_slices < MAX_TIME_SLICES){
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

        currind= PROCX(curr->pid);

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
    if (curr && curr->status != PROC_RUNNABLE) {
        curr = NULL;
    }

    // keep runing the last proc while it exists
    if (curr) {
        switch_proc(curr);
    }

    PANIC("+++++++++++++++++++++ Nothing to run at sched yield!?");    
}