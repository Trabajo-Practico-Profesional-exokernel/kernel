#ifndef INC_MEM_LAYOUT
#define INC_MEM_LAYOUT

#include "arch_inc/mem_constants.h"


#define USER_STACK_PAGE_COUNT 6

// The base virtual address of an application/user proc image. This needs to match the
// starting address defined in `user.ld`.

#define VADDR_USER_BASE 0x1000000

// When compiling we ensure program does not go beyond 0x1800000  
//    ASSERT(. < 0x1800000, "too large executable");... in user.ld
#define VADDR_USER_HARD_END 0x1800000 // The user prog code end could be before this...  For now we just take it as If it always occupied max space

#define VADDR_USER_STACK_HARD_END \
    (VADDR_USER_HARD_END + (USER_STACK_PAGE_COUNT * PAGE_SIZE))

    
// It is basically the place where the trampoline is or so.
// For now its the the ins where the program was loaded

#endif