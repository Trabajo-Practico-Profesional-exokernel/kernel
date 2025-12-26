#include "arch/trap_handling.h"
#include "inc/common.h"
#include "inc/syscalls.h"
#include "inc/filesystem.h"
#include "arch/communication.h"

void syscall_fstat(FullTrapFrame *tf, uintptr_t pc) {
    int fd = SYSCALL_ARG0(tf);
    vaddr_t statbuf_vaddr = SYSCALL_ARG1(tf);

    printf("syscall_fstat called fd=%d statbuf=%x\n", fd, statbuf_vaddr);
    
    SET_SYSCALL_RET0(tf, -1); 
}

void syscall_open(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);
    int flags = SYSCALL_ARG1(tf);
    int mode = SYSCALL_ARG2(tf); //Por si se usa O_CREAT

    printf("syscall_open called path_ptr=%x flags=%x mode=%x\n", path_vaddr, flags, mode);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_mknod(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);
    int major = SYSCALL_ARG1(tf);
    int minor = SYSCALL_ARG2(tf);

    printf("syscall_mknod called path_ptr=%x major=%d minor=%d\n", path_vaddr, major, minor);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_unlink(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);

    printf("syscall_unlink called path_ptr=%x\n", path_vaddr);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_link(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t old_path_vaddr = SYSCALL_ARG0(tf);
    vaddr_t new_path_vaddr = SYSCALL_ARG1(tf);

    printf("syscall_link called old=%x new=%x\n", old_path_vaddr, new_path_vaddr);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_mkdir(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);
    int mode = SYSCALL_ARG1(tf);

    printf("syscall_mkdir called path_ptr=%x mode=%x\n", path_vaddr, mode);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_close(FullTrapFrame *tf, uintptr_t pc) {
    int fd = SYSCALL_ARG0(tf);

    printf("syscall_close called fd=%d\n", fd);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_chdir(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);

    printf("syscall_chdir called path_ptr=%x\n", path_vaddr);
    
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