#include "proc.h"

#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
#include "arch/mem.h" // Declares the methods switch page and so on.
#include "arch/switch.h"// Declares the swtich context to new Proc and sleep method.

#include "arch/mem_layout.h"
#include "arch/mem.h"
 
#include "string.h"
#include "stdlib.h"
#include "console/debug.h"

#include "proc_pages.h"
#include "proc_syscalls.h"
#include "proc_sleeping.h"
#include "proc_fs.h"

// #include "ipc.h"

void free_process(struct Proc * proc){
    
    free_proc_pages(proc);
    reset_proc_uptime(proc);
    
    proc->user_sp_start = 0;
    proc->pde_paddr = 0;

    proc->pc = 0; // Or some default one If so you want!

    proc->status = PROC_FREE; 

    delete_proc_info(proc->pid);
    // Trapframe reset? maybe for security reasons.. but create_process would reset it anyway!    
}


int copy_argv_pointers_from_user(struct Proc * proc, paddr_t* argv_pointers, vaddr_t vaddr_argv){
    paddr_t src_argv_paddr = get_paddr_for(
                                    (uint32_t *) proc->pde_paddr,
                                    vaddr_argv);
    
    if (src_argv_paddr == 0){
        VERBOSE_PRINTF("Invalid vaddr for argv error!!\n");
        return -2;
    }
    vaddr_t* src_argv = (vaddr_t*) src_argv_paddr;
    int argc;

    for(argc = 0; src_argv[argc]; argc++) { // While argv[ind] != 0
        if(argc >= MAXARG) {
            VERBOSE_PRINTF("MORE THAN MAX PARAMS!\n");
            return -1;
        }

        vaddr_t vaddr_arg = src_argv[argc];
        
        paddr_t paddr_arg = get_paddr_for(
                                        (uint32_t *) proc->pde_paddr,
                                        vaddr_arg);
        if (paddr_arg == 0){
            VERBOSE_PRINTF("Invalid vaddr for arg error!!\n");
            return -2;
        }

        // Just for validation... by the way.. this is wrong since arg parameter could be in multiple pages
        // that are not contiguous.. not now though since kalloc does not exist.
        size_t arg_len = strlen((char* ) paddr_arg) + 1; 
        
        if (arg_len > MAX_ARG_LEN){
            VERBOSE_PRINTF("ARG LONGER THAN ALLOWED!\n");
            return -1;            
        }

        VERBOSE_DEBUG_PRINTF_LV(1, "MAPPED PARAM FOR PROGRAM pointer at %x!\n", paddr_arg);
        
        argv_pointers[argc] = paddr_arg;
    }
    
    argv_pointers[argc] = 0;

    return argc;
}


int set_init_parameters_for_proc(struct Proc * proc, char ** argv, paddr_t* sp_out){
    // Copy arguments to the stack of the proc
    // In the future it could be we use instead dynamically allocated pages
    // Not needed for now.
    paddr_t paddr_sp_end= proc->user_sp_start + USER_STACK_PAGE_COUNT * PAGE_SIZE; // Start at the stack top
    paddr_t sp = paddr_sp_end;
    
    uint32_t argc;
    paddr_t argv_pointers[MAXARG];
    for(argc = 0; argv[argc]; argc++) { // While argv[ind] != 0

        if(argc >= MAXARG) {
            VERBOSE_PRINTF("MORE THAN MAX PARAMS!\n");
            return -1;
        }
        VERBOSE_DEBUG_PRINTF_LV(2,"Pushing ARG at 0x%x sp bfr: %p ", argv[argc], sp);
        VERBOSE_DEBUG_PRINTF_LV(2,"'%s'!\n", argv[argc]);
        size_t arg_len = strlen(argv[argc]) + 1; 

        if (arg_len > MAX_ARG_LEN){
            VERBOSE_PRINTF("ARG LONGER THAN ALLOWED!\n");
            return -1;            
        }
        sp -= arg_len;
        sp -= sp % 16; // riscv sp must be 16-byte aligned

        if(sp < proc->user_sp_start){
            VERBOSE_PRINTF("STACK OVERFLOW!!\n");
            return -2;
        }
        VERBOSE_DEBUG_PRINTF_LV(2,"trg aft stack: %p , len: %d ", sp, arg_len);
        memcpy( (void *) sp, (void *) argv[argc], arg_len);
        argv_pointers[argc] = VADDR_USER_STACK_HARD_END- (paddr_sp_end- sp);
        VERBOSE_DEBUG_PRINTF_LV(2,"Copied arg to 0x%x, vaddr: 0x%x", sp, argv_pointers[argc]);
        VERBOSE_DEBUG_PRINTF_LV(2," value: '%s'\n", sp);
    }
    argv_pointers[argc] = 0;

    // Finally push to the stack.. the actually array of pointer i.e argv_pointers
    size_t argv_bytes_size = (argc+1) * sizeof(paddr_t); 
    sp -= argv_bytes_size; // +1 for the extra 0
    sp -= sp % 16;

    if(sp < proc->user_sp_start){
        VERBOSE_PRINTF("STACK OVERFLOW!!\n");
        return -2;
    }

    memcpy((void *) sp, (void *) &argv_pointers[0], argv_bytes_size);
    
    *sp_out = sp;
    return argc;
}
