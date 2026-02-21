 
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "arch/mem_layout.h"
#include "string.h"
#include "stdlib.h"
#include "console/debug.h"

#include "user_pages_alloc.h"
#include "proc.h"

// paddr_t get_paddr_page(uint32_t *page_table, size_t ind_pte);
// paddr_t get_paddr_page_table(uint32_t *page_directory, size_t ind_pde);
// void get_vaddr_indexs(vaddr_t vaddr, size_t* ind_pde, size_t* ind_pte);

int load_paddr_stack_pages(struct Proc* proc, paddr_t * pages_arr){
    size_t curr_page_ind = 0;

    // For now we use the hard limit! we now the stack is in range
    //VADDR_USER_HARD_END == VADDR_USER_STACK_START ... to ... VADDR_USER_STACK_HARD_END
    WALK_MEM_PAGES(
        proc->pde_paddr,
        VADDR_USER_HARD_END , VADDR_USER_STACK_HARD_END,

        /* ON_MISSING_PDE */
        {
            VERBOSE_DEBUG_PRINTF_LV(1, "Missing PDE for stack vaddr %x\n", (VADDR_USER_HARD_END + curr_page_ind* PAGE_SIZE));
            return -1;
        },

        /* ON_MISSING_PTE */
        {
            VERBOSE_DEBUG_PRINTF_LV(1, "Missing PTE for stack vaddr %x\n", (VADDR_USER_HARD_END + curr_page_ind* PAGE_SIZE));
            return -2;            
        },

        /* BODY */
        {
            *(pages_arr + curr_page_ind) = _paddr;
            curr_page_ind+=1;
        }
    );

    return curr_page_ind;
}

vaddr_t copy_pages_code_segment(struct Proc* proc_src, struct Proc* proc_trg){
    size_t curr_page_ind = 0;
    vaddr_t _vaddr = VADDR_USER_BASE;

    // For now we use the hard limit! we now the stack is in range
    //VADDR_USER_HARD_END == VADDR_USER_STACK_START ... to ... VADDR_USER_STACK_HARD_END
    WALK_MEM_PAGES(
        proc_src->pde_paddr,
        VADDR_USER_BASE , VADDR_USER_HARD_END,

        /* ON_MISSING_PDE */
        {
            VERBOSE_DEBUG_PRINTF_LV(1, "Missing PDE for user code vaddr %x\n", _vaddr);
            break;
        },

        /* ON_MISSING_PTE */
        {
            VERBOSE_DEBUG_PRINTF_LV(1,"Missing PTE for user code vaddr %x\n", _vaddr);
            break;            
        },

        /* BODY */
        {
            VERBOSE_DEBUG_PRINTF_LV(1,"Should map _vaddr= 0x%x to _paddr=0x%x\n",_vaddr, _paddr);
            _vaddr+=PAGE_SIZE;
        }
    );

    VERBOSE_DEBUG_PRINTF_LV(1,"Finished in vaddr 0x%x\n", _vaddr);

    return _vaddr;
}
