
#include "inc/common.h"
#include "arch_inc/mem_constants.h"
#include "arch/mem.h"
#include "std/string.h"

#include "arch/trap_handling.h"
#include "sched.h"

#include "user_pages_alloc.h"

#ifdef IS_RISC
    #define KERNEL_PERMISSIONS_ALL (PAGE_R | PAGE_W | PAGE_X)
    #define USER_PERMISSIONS_ALL (PAGE_U | PAGE_R | PAGE_W | PAGE_X)
#else
    #define KERNEL_PERMISSIONS_ALL (I86_PTE_WRITABLE) // I86_PTE_PRESENT  no HACE FALTA! Ya se setea en el map_page.
    #define USER_PERMISSIONS_ALL (I86_PTE_WRITABLE | I86_PTE_USER)
#endif



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





void syscall_sbrk(FullTrapFrame *tf, uintptr_t pc) {
    int page_count = SYSCALL_ARG0(tf);
    struct Proc* caller_proc = get_curr();

    printf("Should alloc page count %d for %u\n", page_count, caller_proc->pid);
    // Switch to kernel pages to be able to alloc pages
    int ret_value = -1; // By def set as error

    switch_to_kernel_tables();
    uint32_t *pde_table = (uint32_t *) caller_proc->pde_paddr;

    paddr_t allocated_page;
    
    ret_value = try_alloc_user_page(&allocated_page);
    
    if(ret_value == 0){
        ret_value = allocated_page; // If all ok then set ret value to paddr of page... i.e
        // NO mem layout for now to make it be incremental and contiguous

        map_page(pde_table, allocated_page, allocated_page, USER_PERMISSIONS_ALL);
    } else {
        printf("Failed alloc page, no memory left!\n");
    }

    // Switch back to proc tables to return to proc
    switch_page_table(pde_table);

    SET_SYSCALL_RET0(tf, ret_value);
}

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
