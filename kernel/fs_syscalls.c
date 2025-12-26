#include "arch/trap_handling.h"
#include "inc/common.h"
#include "inc/syscalls.h"
#include "inc/filesystem.h"

// Handlers stubs
void syscall_fstat(FullTrapFrame *tf, uintptr_t pc) {
    printf("syscall_fstat called\n");
    SET_SYSCALL_RET0(tf, -1); 
}

void syscall_open(FullTrapFrame *tf, uintptr_t pc) {
    printf("syscall_open called\n");
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_mknod(FullTrapFrame *tf, uintptr_t pc) {
    printf("syscall_mknod called\n");
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_unlink(FullTrapFrame *tf, uintptr_t pc) {
    printf("syscall_unlink called\n");
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_link(FullTrapFrame *tf, uintptr_t pc) {
    printf("syscall_link called\n");
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_mkdir(FullTrapFrame *tf, uintptr_t pc) {
    printf("syscall_mkdir called\n");
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_close(FullTrapFrame *tf, uintptr_t pc) {
    printf("syscall_close called\n");
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_chdir(FullTrapFrame *tf, uintptr_t pc) {
    printf("syscall_chdir called\n");
    SET_SYSCALL_RET0(tf, -1);
}


void init_syscalls_filesystem(void) {
    register_syscall(SYS_FS_STAT, syscall_fstat);
    register_syscall(SYS_OPEN, syscall_open);
    register_syscall(SYS_MKNOD, syscall_mknod);
    register_syscall(SYS_UNLINK, syscall_unlink);
    register_syscall(SYS_LINK, syscall_link);
    register_syscall(SYS_MKDIR, syscall_mkdir);
    register_syscall(SYS_CLOSE, syscall_close);
    register_syscall(SYS_CHDIR, syscall_chdir);
}