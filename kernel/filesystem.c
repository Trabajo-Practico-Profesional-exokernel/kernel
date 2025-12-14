#include "inc/filesystem.h"
#include "inc/common.h"
#include "arch/trap_handling.h"
#include "std/string.h"


void syscall_touch(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_pointer = SYSCALL_ARG0(tf);
    printf("Touch got filepath? %p, %d\n", path_pointer, strlen((char *) path_pointer));
    SET_SYSCALL_RET0(tf, 0)

}

void syscall_remove(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_pointer = SYSCALL_ARG0(tf);
    printf("Remove got filepath? %p, %d\n", path_pointer, strlen((char *)path_pointer));
    SET_SYSCALL_RET0(tf, 0)
}

void syscall_stat(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_pointer = SYSCALL_ARG0(tf);
    printf("Stat got filepath? %p, %d\n", path_pointer, strlen((char *)path_pointer));
    SET_SYSCALL_RET0(tf, 0)
}



void syscall_register_handler(FullTrapFrame *tf, uintptr_t pc) {
    printf("SYS REG HANDLER\n" );
    
    vaddr_t handler_pointer = SYSCALL_ARG0(tf);
    struct FilesystemEventsHandler* handler = (struct FilesystemEventsHandler*) handler_pointer; 
    printf("Registering FS handler!? handler_pointer= %p\n", handler_pointer);
    
    printf("buffer at %p, max len %u \n", handler_pointer, handler->buffer, handler->buffer_len);
    SET_SYSCALL_RET0(tf, 0)
}

void init_syscalls_fs(void){
    register_syscall(SYS_FS_TOUCH, syscall_touch);
    register_syscall(SYS_FS_RM, syscall_remove);
    register_syscall(SYS_FS_STAT, syscall_stat);
    register_syscall(SYS_FS_REG_HANDLER, syscall_register_handler);
}