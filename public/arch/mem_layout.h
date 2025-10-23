#ifndef INC_MEM_LAYOUT
#define INC_MEM_LAYOUT

// The base virtual address of an application/user proc image. This needs to match the
// starting address defined in `user.ld`.
#define VADDR_USER_BASE 0x1000000
// It is basically the place where the trampoline is or so.
// For now its the the ins where the program was loaded

#endif