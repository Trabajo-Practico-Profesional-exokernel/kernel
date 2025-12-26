#include "arch/trap_handling.h"
#include "inc/common.h"
#include "inc/syscalls.h"
#include "inc/filesystem.h"
#include "arch/communication.h"
#include "arch/trap_handling.h"
#include "inc/common.h"

// Sched exec , wait and so on...
#include "sched.h"
#include "proc_syscalls.h"
#include "proc.h"
#include "arch/mem.h" // needed for switch to kernel page tables
#include "arch_inc/trap_constants.h"
#include "arch/logging.h"
#include "std/string.h"
#include "arch/communication.h"

#include "meta/apps_info.h"

#define FILESYSTEM_PID 0

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

    struct Proc* actual_proc = get_curr();


    int recv_msg(FullTrapFrame *tf, uintptr_t pc, bool blocking, uint32_t msg_addr);

    printf("syscall_mkdir called path_ptr=%x", path_vaddr);
    int send_msg_success = send_msg(actual_proc, FILESYSTEM_PID, path_vaddr, MSG_SIZE_MAX, FS_TYPE_MKDIR);
    if (send_msg_success) {
        char msg_recv_content[MSG_SIZE_MAX];

        // validar que el mensaje recibido debe ser del filesystem,
        // de otra manera podria recibirse el mensaje de otro proceso ajeno
        int recv_msg_result = recv_msg(tf, pc, true, (uint32_t)&msg_recv_content[0]);
        if (recv_msg_result){
            printf("MKDIR RESPONSE: %s\n", recv_msg_result);
            SET_SYSCALL_RET0(tf, 0);
        } else {
            printf("Error receiving syscall response\n");
            SET_SYSCALL_RET0(tf, -1);
        }

    } else {
        printf("Error sending syscall msg\n");
        SET_SYSCALL_RET0(tf, -1);
    }
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