#include "inc/common.h"
#include "inc/syscalls.h"
#include "arch/trap_handling.h"
#include "arch/logging.h"

#include "arch/stdio.h"


// Sched exec , wait and so on...
#include "sched.h"
#include "proc.h"

#include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  

// meta/gen/apps_meta.c defines this...
extern struct AppBinaryInfo _binary_apps[];


void syscall_exec(FullTrapFrame *tf) {
    int prog_ind = SYSCALL_ARG0(tf);

    if (prog_ind < 0 || prog_ind>= APP_COUNT){
        printf("Invalid exec call ind %d \n", prog_ind);
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)        
        return;
    }
    printf("Should run program at ind %d \n", prog_ind);
    
    struct Proc* proc= get_first_free_proc();
    // It cannot but NULL it throws panic for now but check it anyway for the future!
    if (proc == NULL){
        SET_SYSCALL_RET0(tf, DEF_ERR_CODE)
        return;
    }

    printf("Should run free proc %p \n", proc);
    load_create_process_user(proc, &_binary_apps[prog_ind]);

    // Now do switch? or not? naaa If you want you could wait for it! after ret.
    SET_SYSCALL_RET0(tf, proc->pid)
}





void syscall_putchar(FullTrapFrame *tf) {
    putchar(SYSCALL_ARG0(tf));
}



void syscall_getchar(FullTrapFrame *tf) {
    while (1) {
        long ch = getchar();
        if (ch >= 0) {
            SET_SYSCALL_RET0(tf, ch); // change sys ret vl
            break;
        //} else {
            //printf("No char recv? %x \n", ch);
        }

        // yield or do something? do not stay doing nothing..
    }            
}


#define MAX_SYSCALLS 16

syscall_handler_t syscall_table[MAX_SYSCALLS] = {
    [SYS_PUTCHAR] = syscall_putchar,
    [SYS_GETCHAR] = syscall_getchar,
    [SYS_EXEC] = syscall_exec,
    // ... other handlers
};

uintptr_t handle_syscall(FullTrapFrame *tf, uintptr_t pc) {
    unsigned sysno = SYSCALL_SYSNO(tf);

    if (sysno >= MAX_SYSCALLS || syscall_table[sysno] == NULL) {
        printf("unexpected syscall a3=%x max sysno: %x at pc: %x\n", sysno, MAX_SYSCALLS, pc);
        printTrapFull(tf);
    } else {
        syscall_table[sysno](tf);
    }

    return pc + 4; // skip ecall
}