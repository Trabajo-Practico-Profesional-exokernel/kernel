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

int copy_param_pointers_from_user(struct Proc * proc, paddr_t* param_pointers, paddr_t paddr_pointers){
    if (paddr_pointers == 0){
        VERBOSE_PRINTF("Invalid vaddr for params error!!\n");
        return -2;
    }
    vaddr_t* src_argv = (vaddr_t*) paddr_pointers;
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
        
        param_pointers[argc] = paddr_arg;
    }
    
    param_pointers[argc] = 0;

    return argc;
}


int copy_to_stack_list(paddr_t* item_pointers, 
        char ** list, paddr_t* max_addr, paddr_t min_addr){
    
    uint32_t argc;
    paddr_t sp = *max_addr;

    for(argc = 0; list[argc]; argc++) { // While list[ind] != 0

        if(argc >= MAXARG) {
            VERBOSE_PRINTF("MORE THAN MAX PARAMS!\n");
            return -1;
        }
        VERBOSE_DEBUG_PRINTF_LV(2,"Pushing ARG at 0x%x sp bfr: %p ", list[argc], sp);
        VERBOSE_DEBUG_PRINTF_LV(2,"'%s'!\n", list[argc]);
        size_t arg_len = strlen(list[argc]) + 1; 

        if (arg_len > MAX_ARG_LEN){
            VERBOSE_PRINTF("ARG LONGER THAN ALLOWED!\n");
            return -1;            
        }
        sp -= arg_len;
        sp -= sp % 16; // riscv sp must be 16-byte aligned

        if(sp < min_addr){
            VERBOSE_PRINTF("STACK OVERFLOW!!\n");
            return -2;
        }
        VERBOSE_DEBUG_PRINTF_LV(2,"trg aft stack: %p , len: %d ", sp, arg_len);
        memcpy( (void *) sp, (void *) list[argc], arg_len);
        item_pointers[argc] = VADDR_USER_STACK_HARD_END- (*max_addr- sp);
        VERBOSE_DEBUG_PRINTF_LV(2,"Copied arg to 0x%x, vaddr: 0x%x", sp, item_pointers[argc]);
        VERBOSE_DEBUG_PRINTF_LV(2," value: '%s'\n", sp);
    }


    item_pointers[argc] = 0;

    // Finally push to the stack.. the actually array of pointer i.e item_pointers
    size_t pointers_bytes_size = (argc+1) * sizeof(paddr_t); 
    sp -= pointers_bytes_size; // +1 for the extra 0
    sp -= sp % 16;

    if(sp < min_addr){
        VERBOSE_PRINTF("STACK OVERFLOW!!\n");
        return -2;
    }
    memcpy((void *) sp, (void *) &item_pointers[0], pointers_bytes_size);
    
    *max_addr = sp;


    return argc;
}






