#include "sched.h"

#include "inc/common.h"

#include "arch/logging.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

#include "arch/mem_layout.h"

struct Proc *proc_a;
struct Proc *proc_b;
struct Proc *curr;
uint32_t curr_slices = 0;
#define MAX_TIME_SLICES 45


void switch_proc(struct Proc* next) {
    curr = next;
    
    curr->status = PROC_RUNNING;    
    //printProc(curr);

    switch_page_table(curr->page_table, &curr->stack[sizeof(curr->stack)]);
    switch_context(curr);
}



struct Proc procs[PROCS_MAX]; // All process control structures.

struct Proc * create_process(uint32_t pc) { // pc == entry point == start instruction
    // Find an unused process control structure.
    struct Proc *proc = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++) {
        if (procs[i].status == PROC_FREE) {
            proc = &procs[i];
            break;
        }
    }

    if (!proc)
        PANIC("no free process slots");

    // Save initial pc on proc.
    proc->pc = pc;
    
    // Stack callee-saved registers. These register values will be restored in
    // the first context switch in switch_context. ... init registers basically?
    init_trapframe(proc);

    // For now kernel stack of process... is on the proc struct itself! xv6 does it in a page a virtual memory.. for the future
    proc->tf.sp = (uint32_t)(&proc->stack[SIZE_KERN_STACK]);


    // Initialize memory/pagetables
    paddr_t page_table_addr = alloc_pages(1);
    uint32_t *page_table = (uint32_t *) page_table_addr;
    
    // First map page for page table as direct map
    map_page(page_table,page_table_addr, page_table_addr,  PAGE_R | PAGE_W| PAGE_X);


    // Map base kernel code pages, does not include any allocated pages, like the page_table_addr
    direct_map_range(page_table, 
            get_paddr_kernel_start(),
            get_paddr_kernel_end(),
            PAGE_R | PAGE_W| PAGE_X
            //| PAGE_U // Allow user space to access everything for now. 
            // No.. it does not allow kernel to access U pages.
    );

    proc->page_table = page_table;

    proc->pid = i;
    proc->status = PROC_RUNNABLE;



    return proc;
}

void sched_yield(FullTrapFrame *tf, uintptr_t proc_pc) {
    curr_slices+=1;
    if (curr_slices< MAX_TIME_SLICES){
        return;
    }
    update_trapframe(curr, tf);
    printf("Should preemptive sched! But for now just update curr proc\n");
    printProc(curr);
    curr_slices = 0;

    /*
    curr->status = PROC_RUNNABLE;

    if (curr == proc_a){
        printf("Should switch to PROC SHELL\n");
        //printProc(proc_b);
        curr_slices = 0;
        switch_proc(proc_b);
    } else{
        printf("Should switch to PROC A\n");
        //printProc(proc_a);
        curr_slices = 0;
        switch_proc(proc_a);
    }
    */
}


extern char __user_space_start[], __user_space_end[];

// This is created on the shell.bin.o by the llvm copy thing or so. 
// _binary_ prefix is always there apps_build_shell_bin is the path to the binary basically
// Essentially is where and how big is the niary of the shell app.
extern char _binary_apps_build_shell_bin_start[], _binary_apps_build_shell_bin_size[];


struct Proc * create_process_user(uint32_t proc_entry) { //const void *image, size_t image_size // future!

    struct Proc *proc= create_process(proc_entry);//((uint32_t) user_entry);

    direct_map_range(proc->page_table,
        (paddr_t)__user_space_start,
        (paddr_t)__user_space_end,
        PAGE_U | PAGE_R | PAGE_W | PAGE_X // User space page.
        );
    return proc;
}


struct Proc * load_create_process_user(const void *image, size_t image_size) {
    struct Proc *proc= create_process(VADDR_USER_BASE);//((uint32_t) user_entry);

    // Map user app instruction pages.. i.e load to memory the process
    for (uint32_t off = 0; off < image_size; off += PAGE_SIZE) {
        paddr_t page = alloc_pages(1);

        // Handle the case where the data to be copied is smaller than the
        // page size.
        size_t remaining = image_size - off;
        size_t copy_size = PAGE_SIZE <= remaining ? PAGE_SIZE : remaining;

        // Fill and map the page.
        memcpy((void *) page, image + off, copy_size);
        map_page(proc->page_table, VADDR_USER_BASE + off, page,
                 PAGE_U | PAGE_R | PAGE_W | PAGE_X);
    }

    return proc;
}

void main_app_a();
void init_sched(void) {
    curr = NULL;
    //proc_a = create_process((uint32_t) proc_a_entry);
    proc_a = create_process_user((uint32_t) main_app_a);
    
    proc_b = load_create_process_user(_binary_apps_build_shell_bin_start, (size_t) _binary_apps_build_shell_bin_size);
    
    //proc_b = create_process_user((uint32_t) main_app_b);
    //proc_b = create_process((uint32_t) proc_b_entry);
    printf("AT CREATE PROCESS A expected pc= %x, ", (uint32_t) main_app_a);
    printProc(proc_a);
    
    printf("AT CREATE PROCESS SHELL expected pc= %x, ", (uint32_t)VADDR_USER_BASE);
    printProc(proc_b);
    // Start proc_a!
    switch_proc(proc_b);
    PANIC("unreachable here!");
}