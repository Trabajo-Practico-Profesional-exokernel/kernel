#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "arch/mem_layout.h"
#include "string.h"
#include "stdlib.h"
#include "constants.h"
#include "arch/trap_handling.h"
#include "sched.h"
#include "stdio.h"
#include "console/debug.h"

#include "proc_pages.h"
#include "user_pages_alloc.h"

#include "arch/arch_init.h"

struct UserProcPages procs_pages[PROCS_MAX]; 

///
/// proc_pages.h/ proc pages handling perse
///

int copy_mem_pages(struct Proc* src_proc, struct Proc* trg_proc){
    printf("=>Copying process code segment!\n");
    struct UserProcPages* src_proc_pages_info = &procs_pages[src_proc->pid];
    uint32_t * trg_pde = (uint32_t *) trg_proc->pde_paddr;

    vaddr_t _vaddr = VADDR_USER_BASE;


    WALK_MEM_PAGES(
        src_proc->pde_paddr,
        VADDR_USER_BASE , VADDR_USER_HARD_END,
        /* ON_MISSING_PDE */
        {
            printf("Missing PDE for user code vaddr %x\n", _vaddr);
            break;
        },

        /* ON_MISSING_PTE */
        {
            printf("Missing PTE for user code vaddr %x\n", _vaddr);
            break;
        },

        /* BODY */
        {

            paddr_t copy_page_paddr = alloc_pages(1); // Non user alloc that is not freeable for now!
            if(copy_page_paddr == 0){
                return -1;
            }
            // printf("Should copy user code page  %u src_paddr: 0x%x trg paddr: 0x%x vaddr: 0x%x\n", src_proc->pid, _paddr, copy_page_paddr, _vaddr);

            memcpy((void *) copy_page_paddr, (void *) _paddr, PAGE_SIZE);
            map_page(trg_pde , _vaddr, copy_page_paddr,
                     USER_PERMISSIONS_ALL);

            _vaddr+=PAGE_SIZE;

        }
    );    

    // If it was dynamic then stack page would be not needed to set to hardlimit!
    _vaddr = VADDR_USER_HARD_END;

    printf("=>Copying process stack!\n");
    
    WALK_MEM_PAGES(
        src_proc->pde_paddr,
        VADDR_USER_HARD_END , VADDR_USER_STACK_HARD_END,
        /* ON_MISSING_PDE */
        {
            printf("Missing PDE for proc stack vaddr 0x%x\n", _vaddr);
            break;
        },

        /* ON_MISSING_PTE */
        {
            printf("Missing PTE for proc stack vaddr 0x%x\n", _vaddr);
            break;
        },

        /* BODY */
        {
            paddr_t copy_page_paddr = alloc_pages(1); // Non user alloc that is not freeable for now!
            if(copy_page_paddr == 0){
                return -1;
            }
            // printf("Should copy user stack page  %u src_paddr: 0x%x trg paddr: 0x%x vaddr: 0x%x\n", src_proc->pid, _paddr, copy_page_paddr, _vaddr);

            memcpy((void *) copy_page_paddr, (void *) _paddr, PAGE_SIZE);
            map_page(trg_pde , _vaddr, copy_page_paddr,
                     USER_PERMISSIONS_ALL);

            _vaddr+=PAGE_SIZE;

        }
    );    

    // If it was dynamic then it would be not needed to set to hardlimit!
    _vaddr = VADDR_USER_HEAP_START;

    printf("=>Copying process heap!\n");
    

    WALK_MEM_PAGES(
        src_proc->pde_paddr,
        VADDR_USER_HEAP_START , src_proc_pages_info->vaddr_proc_heap_end,
        /* ON_MISSING_PDE */
        {
            printf("Missing PDE for proc heap vaddr 0x%x\n", _vaddr);
            break;
        },

        /* ON_MISSING_PTE */
        {
            printf("Missing PTE for proc heap vaddr 0x%x\n", _vaddr);
            break;
        },

        /* BODY */
        {
            paddr_t copy_page_paddr = alloc_pages(1); // Non user alloc that is not freeable for now!
            if(copy_page_paddr == 0){
                return -1;
            }
            printf("Should copy user heap page  %u src_paddr: 0x%x trg paddr: 0x%x vaddr: 0x%x\n", src_proc->pid, _paddr, copy_page_paddr, _vaddr);

            memcpy((void *) copy_page_paddr, (void *) _paddr, PAGE_SIZE);
            map_page(trg_pde , _vaddr, copy_page_paddr,
                     USER_PERMISSIONS_ALL);
            _vaddr+=PAGE_SIZE;
        }
    );     

    struct UserProcPages* trg_pages_info = &procs_pages[trg_proc->pid];
    *trg_pages_info = *src_proc_pages_info;
    
    return 0;
}

void free_proc_pages(struct Proc* proc){   
    struct UserProcPages* proc_page_info = &procs_pages[proc->pid];

    // First free user code segment
    vaddr_t _vaddr = VADDR_USER_BASE;

    printf("=>Freeing process code segment!\n");
    WALK_MEM_PAGES(
        proc->pde_paddr,
        VADDR_USER_BASE , VADDR_USER_HARD_END,
        /* ON_MISSING_PDE */
        {
            printf("Missing PDE for user code vaddr %x\n", _vaddr);
            break;
        },

        /* ON_MISSING_PTE */
        {
            printf("Missing PTE for user code vaddr %x\n", _vaddr);
            break;
        },

        /* BODY */
        {
            printf("Should free user code page  %u paddr: 0x%x vaddr: 0x%x\n", proc->pid, _paddr, _vaddr);
            _vaddr+=PAGE_SIZE;

        }
    );    

    // If it was dynamic then stack page would be not needed to set to hardlimit!
    _vaddr = VADDR_USER_HARD_END;

    printf("=>Freeing process stack!\n");
    
    WALK_MEM_PAGES(
        proc->pde_paddr,
        VADDR_USER_HARD_END , VADDR_USER_STACK_HARD_END,
        /* ON_MISSING_PDE */
        {
            printf("Missing PDE for proc stack vaddr 0x%x\n", _vaddr);
            break;
        },

        /* ON_MISSING_PTE */
        {
            printf("Missing PTE for proc stack vaddr 0x%x\n", _vaddr);
            break;
        },

        /* BODY */
        {
            printf("Should free proc stack page  %u paddr: 0x%x vaddr: 0x%x\n", proc->pid, _paddr, _vaddr);
            _vaddr+=PAGE_SIZE;

        }
    );    

    // If it was dynamic then it would be not needed to set to hardlimit!
    _vaddr = VADDR_USER_HEAP_START;

    printf("=>Freeing process heap!\n");
    

    WALK_MEM_PAGES(
        proc->pde_paddr,
        VADDR_USER_HEAP_START , proc_page_info->vaddr_proc_heap_end,
        /* ON_MISSING_PDE */
        {
            printf("Missing PDE for proc heap vaddr 0x%x\n", _vaddr);
            break;
        },

        /* ON_MISSING_PTE */
        {
            printf("Missing PTE for proc heap vaddr 0x%x\n", _vaddr);
            break;
        },

        /* BODY */
        {
            printf("Should free proc heap page  %u paddr: 0x%x vaddr: 0x%x\n", proc->pid, _paddr, _vaddr);
            _vaddr+=PAGE_SIZE;
        }
    );    
    
}




void init_proc_pages(struct Proc* proc){

    // Alloc user stack... still on non user alloc
    proc->user_sp_start = alloc_pages(USER_STACK_PAGE_COUNT);
    debug_printf("FOR PROC %u USER SP_START IS %x \n", proc->pid, proc->user_sp_start);

    //
    // Map user stack to vaddr
    // [USER_PROG_HARD_END ..USER_STACK_PAGE_COUNT .. USER_STACK_HARD_END] ..    
    // ... to avoid having to calculate dynamic start. Also ... no page for safeguard yet. 
    //
    paddr_t paddr_sp_end = proc->user_sp_start + USER_STACK_PAGE_COUNT * PAGE_SIZE;
    
    offset_map_range((uint32_t *) proc->pde_paddr, 
            proc->user_sp_start, paddr_sp_end,
            USER_PERMISSIONS_ALL, VADDR_USER_HARD_END); 


    struct UserProcPages* proc_pages = &procs_pages[proc->pid];
    proc_pages->vaddr_proc_heap_end = VADDR_USER_HEAP_START; // For now hardcoded hardlimit.. no dynamic setting.
}

paddr_t get_paddr_proc_heap_page(struct Proc * proc, size_t ind){
    vaddr_t trg_vaddr = VADDR_USER_HEAP_START + ind * PAGE_SIZE;
     
    struct UserProcPages* proc_pages = &procs_pages[proc->pid];
    if(trg_vaddr >= proc_pages->vaddr_proc_heap_end){
        return 0; // Not mapped/used for proc.
    }
    
    return get_paddr_for((uint32_t *) proc->pde_paddr, trg_vaddr);
}




int try_add_heap_page_for_proc(struct Proc * proc, paddr_t* allocated_paddr){
    struct UserProcPages* proc_pages = &procs_pages[proc->pid];

    if (proc_pages->vaddr_proc_heap_end >= VADDR_USER_HEAP_HARD_END){
        debug_printf("Process %u reached max limit of heap pages! Cannot alloc\n", proc->pid);
        return -1;
    }

    int ret_value = try_alloc_user_page(allocated_paddr);
    
    if(ret_value < 0){
        debug_printf("Failed alloc page, no memory left!\n");
        return ret_value;
    }

    vaddr_t vaddr_new_page =proc_pages->vaddr_proc_heap_end;
    proc_pages->vaddr_proc_heap_end+= PAGE_SIZE;
    
    map_page((uint32_t *) proc->pde_paddr, vaddr_new_page, *allocated_paddr, USER_PERMISSIONS_ALL);
    
    // offset/index of page
    return (vaddr_new_page- VADDR_USER_HEAP_START)/ PAGE_SIZE;
}

// VADDR_USER_HEAP_START, VADDR_USER_HEAP_HARD_END
vaddr_t get_vaddr_user_heap_page(size_t ind){
    return VADDR_USER_HEAP_START + ind * PAGE_SIZE;
}


///
/// Syscalls
///
void syscall_sbrk(FullTrapFrame *tf, uintptr_t pc) {
    int page_count = SYSCALL_ARG0(tf);
    struct Proc* caller_proc = get_curr();

    debug_printf("Should alloc page count %d for %u\n", page_count, caller_proc->pid);
    // Switch to kernel pages to be able to alloc pages
    int ret_value = -1; // By def set as error

    switch_to_kernel_tables();
    uint32_t *pde_table = (uint32_t *) caller_proc->pde_paddr;

    paddr_t allocated_page;
    
    ret_value = try_add_heap_page_for_proc(caller_proc, &allocated_page);
    
    if(ret_value >= 0){
        vaddr_t ret_vaddr = get_vaddr_user_heap_page(ret_value);
        debug_printf("SBRK incremented by 1! Allocated page ind %d, vaddr= %x to paddr= %x\n", ret_value, ret_vaddr, allocated_page);
        ret_value = ret_vaddr;
    }

    // Switch back to proc tables to return to proc
    switch_page_table(pde_table);

    SET_SYSCALL_RET0(tf, ret_value);
}



void init_proc_mem_management(void) {
    init_user_pages_alloc();

    /// Now register syscalls!
    register_syscall(SYS_SBRK, syscall_sbrk);

}
