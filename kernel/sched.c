#include "sched.h"

#include "inc/common.h"

#include "arch/logging.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

void switch_proc(struct Proc* next) {
    next->status = PROC_RUNNING;    
    //switch_page_table(next->page_table, &next->stack[sizeof(next->stack)]);
    switch_context(next);
}



struct Proc procs[PROCS_MAX]; // All process control structures.

struct Proc * create_process(uint32_t pc) { // pc == entry point == start instruction
    // Find an unused process control structure.
    struct Proc *proc = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++) {
        if (procs[i].status == PROC_FREE) {
            proc = &procs[i];
            //memset(proc, 0, sizeof(struct Proc));  // just in case.. delete any garbage values
            break;
        }
    }

    if (!proc)
        PANIC("no free process slots");


    // Stack callee-saved registers. These register values will be restored in
    // the first context switch in switch_context. ... init registers basically?
    init_trapframe(proc, (uint32_t) pc);

    // For now kernel stack of process... is on the proc struct itself! xv6 does it in a page a virtual memory.. for the future
    proc->tf.sp = (uint32_t)(&proc->stack[SIZE_KERN_STACK]);


    // Initialize memory/pagetables
    uint32_t *page_table = (uint32_t *) alloc_pages(1);

    paddr_t start = get_paddr_page_ind(0); // physical address first page.

    //direct_map_all_pages(page_table, start, PAGE_R | PAGE_W | PAGE_X);
    paddr_t second = direct_map_n_pages(page_table,start, 2, PAGE_R| PAGE_X);
    second = offset_map_n_pages(page_table,start, second -start // Offset one page in vaddr
                        , 2, PAGE_R);
    direct_map_all_pages(page_table,second, PAGE_R | PAGE_W| PAGE_X);

    proc->page_table = page_table;

    proc->pid = i;
    proc->status = PROC_RUNNABLE;



    return proc;
}

#define SLEEP_TIME 300000000

struct Proc *proc_a;
struct Proc *proc_b;
struct Proc *curr;
uint32_t curr_slices = 0;
#define MAX_TIME_SLICES 15


// No preemptive scheduling yet!

void proc_a_entry(void) {
    printf("starting process A\n");
    //syscall(SYS_KALLOC, 1, 0, 0); //For when its on user space.
    //printf("called kalloc on A\n");
    while (1) {
        sleep(SLEEP_TIME);
        printf("A after sleep\n");
        //printProc(proc_a);
    }
}

void proc_b_entry(void) {
    printf("starting process B\n");
    while (1) {
        sleep(SLEEP_TIME);
        printf("Inbetween next iter on proc_b!\n");
        sleep(SLEEP_TIME);
        //switch_proc(proc_b, proc_a);
        printf("B after sleep proc_b: \n");
        //printProc(proc_b);
    }
}

void init_sched(void) {
    curr = NULL;
    proc_a = create_process((uint32_t) proc_a_entry);
    proc_b = create_process((uint32_t) proc_b_entry);
    printf("AT CREATE PROCESS A expected pc= %u, ", (uint32_t) proc_a_entry);
    printProc(proc_a);
    
    printf("AT CREATE PROCESS B expected pc= %u, ", (uint32_t) proc_b_entry);
    printProc(proc_b);
    curr = proc_a;
    
    // Start proc_a!
    switch_proc(proc_a);
    PANIC("unreachable here!");
}

void sched_yield(FullTrapFrame *tf) {
    curr_slices+=1;
    if (curr_slices< MAX_TIME_SLICES){
        return;
    }
    update_trapframe(curr, tf);
    curr->status = PROC_RUNNABLE;

    if (curr == proc_a){
        printf("Should switch to PROC B\n");
        //printProc(proc_b);
        curr_slices = 0;
        curr= proc_b;
        switch_proc(proc_b);
    } else{
        printf("Should switch to PROC A\n");
        //printProc(proc_a);
        curr_slices = 0;
        curr= proc_a;
        switch_proc(proc_a);
    }

    //curr_slices=0;
    //printf("Preemtptive sched!\n");
}
