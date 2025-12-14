#include "inc/filesystem.h"
#include "inc/common.h"
#include "arch/trap_handling.h"
#include "std/string.h"


void syscall_touch(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t name_pointer = SYSCALL_ARG0(tf);
    printf("Touch got filename? %p, %d\n", name_pointer, strlen((char *) name_pointer));
}

void syscall_remove(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t name_pointer = SYSCALL_ARG0(tf);
    printf("Remove got filename? %p, %d\n", name_pointer, strlen((char *)name_pointer));
}

void init_syscalls_fs(void){
    register_syscall(SYS_TOUCH, syscall_touch);
    register_syscall(SYS_RM, syscall_remove);
}
