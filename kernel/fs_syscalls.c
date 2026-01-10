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


void syscall_fstat(FullTrapFrame *tf, uintptr_t pc) {
    int fd = SYSCALL_ARG0(tf);
    vaddr_t statbuf_vaddr = SYSCALL_ARG1(tf);

    debug_printf("syscall_fstat called fd=%d statbuf=%x\n", fd, statbuf_vaddr);
    
    SET_SYSCALL_RET0(tf, -1); 
}

void syscall_open(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);
    int flags = SYSCALL_ARG1(tf);
    int mode = SYSCALL_ARG2(tf); //Por si se usa O_CREAT

    debug_printf("syscall_open called path_ptr=%x flags=%x mode=%x\n", path_vaddr, flags, mode);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_mknod(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);
    int major = SYSCALL_ARG1(tf);
    int minor = SYSCALL_ARG2(tf);

    debug_printf("syscall_mknod called path_ptr=%x major=%d minor=%d\n", path_vaddr, major, minor);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_unlink(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);

    debug_printf("syscall_unlink called path_ptr=%x\n", path_vaddr);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_link(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t old_path_vaddr = SYSCALL_ARG0(tf);
    vaddr_t new_path_vaddr = SYSCALL_ARG1(tf);

    debug_printf("syscall_link called old=%x new=%x\n", old_path_vaddr, new_path_vaddr);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_mkdir(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);

    struct Proc* actual_proc = get_curr();

    debug_printf("syscall_mkdir called path_ptr=%x", path_vaddr);
    // switch_to_kernel_tables();// You need to be on kernel pages to be able to map/get real paddr
    
    int send_msg_success = send_msg(actual_proc, filesystem_PID, path_vaddr, FS_TYPE_MKDIR);
    if (send_msg_success) {
        char msg_recv_content[MSG_SIZE_MAX];

        // validar que el mensaje recibido debe ser del filesystem,
        // de otra manera podria recibirse el mensaje de otro proceso ajeno
        int recv_msg_result = recv_msg(tf, pc, true, (uint32_t)&msg_recv_content[0]);
        if (recv_msg_result >= 0){
            debug_printf("MKDIR RESPONSE: %x\n", msg_recv_content[0]);
            if ((uint32_t)msg_recv_content[0] >= 0){
                SET_SYSCALL_RET0(tf, 0);
            } else {
                SET_SYSCALL_RET0(tf, -1);
            }
        } else {
            debug_printf("Error receiving syscall response\n");
            SET_SYSCALL_RET0(tf, -1);
        }
    } else {
        debug_printf("Error sending syscall msg\n");
        SET_SYSCALL_RET0(tf, -1);
    }
}


void syscall_rmdir(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);

    struct Proc* actual_proc = get_curr();

    debug_printf("syscall_rmdir called path_ptr=%x", path_vaddr);
    // switch_to_kernel_tables();// You need to be on kernel pages to be able to map/get real paddr
    
    int send_msg_success = send_msg(actual_proc, filesystem_PID, path_vaddr, FS_TYPE_RMDIR);
    
    if (send_msg_success) {
        char msg_recv_content[MSG_SIZE_MAX];

        // validar que el mensaje recibido debe ser del filesystem,
        // de otra manera podria recibirse el mensaje de otro proceso ajeno
        int recv_msg_result = recv_msg(tf, pc, true, (uint32_t)&msg_recv_content[0]);
        if (recv_msg_result >= 0){
            debug_printf("RMDIR RESPONSE: %x\n", (uint32_t)msg_recv_content[0]);
            if ((uint32_t)msg_recv_content[0] >= 0){
                SET_SYSCALL_RET0(tf, 0);
            } else {
                SET_SYSCALL_RET0(tf, -1);
            }
        } else {
            debug_printf("Error receiving syscall response\n");
            SET_SYSCALL_RET0(tf, -1);
        }
    } else {
        debug_printf("Error sending syscall msg\n");
        SET_SYSCALL_RET0(tf, -1);
    }
}

void syscall_close(FullTrapFrame *tf, uintptr_t pc) {
    int fd = SYSCALL_ARG0(tf);

    debug_printf("syscall_close called fd=%d\n", fd);
    
    SET_SYSCALL_RET0(tf, -1);
}

void syscall_chdir(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t path_vaddr = SYSCALL_ARG0(tf);

    debug_printf("syscall_chdir called path_ptr=%x\n", path_vaddr);
    
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
    register_syscall(SYS_FS_RM, syscall_rmdir);
}