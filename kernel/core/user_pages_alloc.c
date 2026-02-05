
 
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

size_t get_free_user_memory(void) {
    size_t count = 0;
    free_page_t *curr = free_list_head;

    while (curr != NULL) {
        count++;
        curr = curr->next;
    }

    return count * PAGE_SIZE;
}

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
}
