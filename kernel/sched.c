#include "sched.h"
#include "proc.h"

#include "inc/common.h"
#include "arch/logging.h"

//#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

#include "arch/mem_layout.h"

#define MAX_TIME_SLICES 45
uint32_t curr_slices = 0;

// For now, for simple switching. Not even round robin for a vec lol.
struct Proc *proc_a;
struct Proc *proc_b;

void set_proc_a(struct Proc * proc){
    proc_a = proc;
}
void set_proc_b(struct Proc * proc){
    proc_b = proc;    
}


struct Proc *curr;

void set_curr(struct Proc * proc){
    curr = proc;    
}


struct Proc procs[PROCS_MAX]; // All process control structures.

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
    
    curr->status = PROC_RUNNING;


    printf("[NEXT PROC BEFORE SWITCH_CONTEXT] ");
    printProc(next);
    printf("\n\n");

    #ifdef IS_RISC
    SWITCH_TO_STACK(&trampoline_stacks[curr->cpunum][TRAMPOLINE_STACK_SIZE])
    
    switch_page_table(curr->page_table, (uint8_t *) curr->kernel_sp);
    #else
    // TODO: llamar switch_page_table para lo de cr3
    switch_page_table(curr->pde_paddr);

    #endif



    switch_context(curr);
}

void sched_yield(FullTrapFrame *tf, uintptr_t proc_pc) {
    curr_slices+=1;
    if (curr_slices< MAX_TIME_SLICES){
        return;
    }
    update_trapframe(curr, tf);
    #ifdef IS_RISC
    curr_slices = 0;
    
    //printf("Should preemptive sched! But for now just update curr proc\n");
    //printProc(curr);

    curr->status = PROC_RUNNABLE;

    if (curr == proc_a){
        printf("Should switch to PROC SHELL\n");
        switch_proc(proc_b);
    } else{
        printf("Should switch to PROC A\n");
        switch_proc(proc_a);
    }
    #else // IS X86
    
    curr->status = PROC_RUNNABLE;
    curr_slices = 0;

    printf("[PROC RUNNING] ");
    printProc(curr);

    if (curr == proc_a){
        printf("Should switch to PROC B\n");
        switch_proc(proc_b);
    } else{
        printf("Should switch to PROC A\n");
        switch_proc(proc_a);
    }
    printf("Preemtptive sched!\n");
    #endif
}