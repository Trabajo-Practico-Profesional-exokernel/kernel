
 
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "arch/mem_layout.h"
#include "std/string.h"
#include "constants.h"
#include "arch/trap_handling.h"
#include "sched.h"
#include "std/printf.h"
#include "console/debug.h"


#include "user_pages_alloc.h"

///
/// Paddr linked list manager for the user ram memory space i.e user_pages_alloc.h contract
///
extern char __user_ram_start[], __user_ram_end[];

typedef struct free_page {
    struct free_page *next;
} free_page_t;


// #define PAGE_INDEX(paddr) (((paddr) - (paddr_t)__user_ram_start) / PAGE_SIZE);


free_page_t *free_list_head = NULL;
free_page_t *first_page = NULL;
free_page_t *last_page = NULL;
// size_t free_pages_count = 0;

bool has_no_user_free_pages(void){
    return free_list_head == NULL;
}

size_t user_pages_count(void){
    return ((paddr_t)__user_ram_end -(paddr_t)__user_ram_start ) / PAGE_SIZE;
}

// Pop first free page
paddr_t alloc_user_page(void) {
    if (!free_list_head) // Just in case
        PANIC("out of memory");

    free_page_t *page = free_list_head;
    free_list_head = page->next;
    // free_pages_count-=1;

    memset(page, 0, PAGE_SIZE);

    return (paddr_t)page;
}

int try_alloc_user_page(paddr_t * out_paddr) {
    if (!free_list_head) // Just in case
        return -1;

    free_page_t *page = free_list_head;
    free_list_head = page->next;
    // free_pages_count-=1;

    memset(page, 0, PAGE_SIZE);
    *out_paddr = (paddr_t)page;
    
    return 0;
}



// Push the freed page as new free head... no need to keep order
// Last page freed will be the first to be allocated.
void free_user_page(paddr_t paddr) { //
    free_page_t *page = (free_page_t *)paddr;
    page->next = free_list_head;
    free_list_head = page;

    // free_pages_count+=1;
}


paddr_t get_paddr_user_page_ind(uint32_t ind){
    return ((paddr_t) __user_ram_start) + (PAGE_SIZE * ind);
}

paddr_t get_paddr_user_last_page(){
    return (paddr_t) last_page;
}


///
/// proc.h/ proc pages handling perse
///

// VADDR_USER_HEAP_START, VADDR_USER_HEAP_HARD_END
vaddr_t get_vaddr_user_heap_page(size_t ind){
    return VADDR_USER_HEAP_START + ind * PAGE_SIZE;
}

struct UserProcPages {
    paddr_t vaddr_proc_heap_end;
    // paddr_t vaddr_proc_code_end; // PODRIAN SER DINAMICOS! En vez de estaticos!
    // paddr_t vaddr_proc_stack_end;
};

struct UserProcPages procs_pages[PROCS_MAX]; 


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

void free_proc_pages(struct Proc* proc){   
    struct UserProcPages* proc_pages = &procs_pages[proc->pid];

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
        VADDR_USER_HEAP_START , proc_pages->vaddr_proc_heap_end,
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






///
/// Init method
///

void init_user_pages_alloc(void) {
    paddr_t start = (paddr_t)__user_ram_start;
    paddr_t end   = (paddr_t)__user_ram_end;


    last_page = (free_page_t *) (end- PAGE_SIZE); // Last page == highest addr 

    // Build linked list of free pages. i.e structure available ram into pages.
    for (paddr_t curr_free_page = (paddr_t) last_page; curr_free_page >= start; curr_free_page -= PAGE_SIZE) {
        free_page_t *page = (free_page_t *) curr_free_page;

        // lower page points to higher page...
        // ex: page at 1 =next=> page at 2 ... free list is decreasing starting from last one.
        // So it points to the next page in addr order.
        page->next = free_list_head;

        free_list_head = page;
        // free_pages_count+=1;
    }

    first_page = free_list_head; // At the end free list points to first page lowest addr.
    /// Now register syscalls!
    register_syscall(SYS_SBRK, syscall_sbrk);

}
