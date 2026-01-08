#include "inc/filesystem.h"
#include "inc/common.h"
#include "arch/trap_handling.h"
#include "std/string.h"

#include "sched.h"
#include "arch/mem.h" // needed for switch to kernel page tables



struct Proc * fs_manager_proc;
void * event_buffer;
size_t buffer_max_len;
struct FilesystemEventsHandler* fs_handler = NULL;

struct Proc* curr_caller = NULL; // For now just a single one. Always one since its 1 req per time.
// In the future there will be a queue of events to be sent.


int copy_valid_bytes(vaddr_t bytes_pointer, struct Proc * caller){
    if (fs_handler == NULL){
        debug_printf("Failed there is no registered FS handler!\n");
        return -2;
    }

    paddr_t real_pointer = get_paddr_for((uint32_t *) caller->pde_paddr,
                                    bytes_pointer 
                                    );

    if (real_pointer == 0){
        debug_printf("Failed invalid vaddr for name = %x not mapped for proc %d\n", bytes_pointer, caller->pid);
        return -1;
    }
    size_t len_name = strlen((char *) real_pointer);

    if (len_name > buffer_max_len){
        debug_printf("Failed path name was too long %u, max allowed is %u \n", len_name, buffer_max_len);
        return -2;
    }

    strncpy((char*)event_buffer, (char*)real_pointer, len_name);

    return len_name;
}

void syscall_handle_ret(FullTrapFrame *tf, uintptr_t pc) {
    if (curr_caller == NULL){
        debug_printf("Filesystem or other proc called fs ret but no current caller yielding \n");
        sched_yield();
    }
    struct Proc * returner = get_curr();

    if (fs_manager_proc != returner){
        debug_printf("Non fs manager proc called fs ret.. yielding \n");
        sched_yield();
    }
    returner->status = PROC_NOT_RUNNABLE;

    curr_caller->status = PROC_RUNNABLE;

    int ret_code = SYSCALL_ARG0(tf);

    // SET_SYSCALL_RET0(curr_caller->tf, ret_code) // When returning it will have the ret code. That could aswell be fd or so
    save_curr_proc_state(tf, pc);
    
    struct TrapFrame* caller_tf = &curr_caller->tf;
    SET_SYSCALL_RET0(caller_tf, ret_code);

    debug_printf("Fs manager returned %d to caller %d \n",ret_code, curr_caller->pid);
    curr_caller = NULL;
    sched_yield();

}


void syscall_touch(FullTrapFrame *tf, uintptr_t pc) {
    struct Proc * caller = get_curr();
    vaddr_t vaddr_bytes_pointer = SYSCALL_ARG0(tf);
    #ifdef IS_RISC
    switch_to_kernel_tables();// You need to be on kernel pages to be able to map/get real paddr
    #endif

    debug_printf("Touch ");
    int err = copy_valid_bytes(vaddr_bytes_pointer, caller);

    if (err < 0){
      SET_SYSCALL_RET0(tf, err)
      return;    
    }
    caller->status = PROC_NOT_RUNNABLE;
    curr_caller = caller;

    fs_manager_proc->status = PROC_RUNNING;
    fs_manager_proc->pc = (uintptr_t) fs_handler->on_touch; // Set where to jump back to
    debug_printf("Handling... path '%s' .. jumping to %x\n", (char*) event_buffer, fs_manager_proc->pc);
    
    save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall

    switch_proc(fs_manager_proc);

}

void syscall_remove(FullTrapFrame *tf, uintptr_t pc) {
    struct Proc * caller = get_curr();
    vaddr_t vaddr_bytes_pointer = SYSCALL_ARG0(tf);
    #ifdef IS_RISC
    switch_to_kernel_tables();// You need to be on kernel pages to be able to map/get real paddr
    #endif

    debug_printf("Remove ");
    int err = copy_valid_bytes(vaddr_bytes_pointer, caller);

    if (err < 0){
      SET_SYSCALL_RET0(tf, err)
      return;    
    }
    caller->status = PROC_NOT_RUNNABLE;
    curr_caller = caller;

    fs_manager_proc->status = PROC_RUNNING;
    fs_manager_proc->pc = (uintptr_t) fs_handler->on_rm; // Set where to jump back to
    debug_printf("Handling... path '%s' .. jumping to %x\n", (char*) event_buffer, fs_manager_proc->pc);
    
    save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall

    switch_proc(fs_manager_proc);

}

void syscall_stat(FullTrapFrame *tf, uintptr_t pc) {
    struct Proc * caller = get_curr();
    vaddr_t vaddr_bytes_pointer = SYSCALL_ARG0(tf);
    #ifdef IS_RISC
    switch_to_kernel_tables();// You need to be on kernel pages to be able to map/get real paddr
    #endif

    debug_printf("Stat ");
    int err = copy_valid_bytes(vaddr_bytes_pointer, caller);

    if (err < 0){
      SET_SYSCALL_RET0(tf, err)
      return;    
    }
    caller->status = PROC_NOT_RUNNABLE;
    curr_caller = caller;

    fs_manager_proc->status = PROC_RUNNING;
    fs_manager_proc->pc = (uintptr_t) fs_handler->on_stat; // Set where to jump back to
    debug_printf("Handling... path '%s' .. jumping to %x\n", (char*) event_buffer, fs_manager_proc->pc);
    
    save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall

    switch_proc(fs_manager_proc);

}



void syscall_register_handler(FullTrapFrame *tf, uintptr_t pc) {
    
    if (fs_handler != NULL){
        debug_printf("Already registered FS handler! cannot have another one!\n");
        SET_SYSCALL_RET0(tf, -2)
        return;
    }
    
    vaddr_t handler_pointer = SYSCALL_ARG0(tf);
    #ifdef IS_RISC
    switch_to_kernel_tables();// You need to be on kernel pages to be able to map/get real paddr
    #endif

    struct Proc * proc = get_curr();

    // debug_printf("Registering FS handler!? vaddr handler_pointer= %x in curr proc %d\n", handler_pointer, proc->pid);

    paddr_t real_paddr = get_paddr_for((uint32_t *) proc->pde_paddr,
                                    handler_pointer 
                                    );
    if (real_paddr == 0){
        debug_printf("Registering FS handler failed invalid vaddr handler_pointer= %x not mapped for proc %d\n", handler_pointer, proc->pid);
        // Invalid or not accessible for user proc vaddr
        SET_SYSCALL_RET0(tf, -1);
        return;
    }

    struct FilesystemEventsHandler* handler = (struct FilesystemEventsHandler*) real_paddr; 
    // debug_printf("real paddr= %x pointer? %p\n", real_paddr, handler);

    if(handler->buffer_len > PAGE_SIZE){
        debug_printf("Registering FS handler failed invalid buffer max len %u greater than a page size\n", handler->buffer_len);
        // MAX buffer len allowed is page size just for convinience
        SET_SYSCALL_RET0(tf, -3);
        return;        
    }
    paddr_t real_buffer = get_paddr_for((uint32_t *) proc->pde_paddr,
                                    (vaddr_t) handler->buffer 
                                    );

    if (real_buffer == 0){
        debug_printf("Registering FS handler failed invalid vaddr buffer= %x not mapped for proc %d\n", handler->buffer, proc->pid);
        // Invalid or not accessible for buffer vaddr
        SET_SYSCALL_RET0(tf, -1);
        return;
    }

    // Set the handler 
    fs_manager_proc = proc;
    event_buffer = (void *)real_buffer;
    buffer_max_len = handler->buffer_len;
    fs_handler = handler;
    debug_printf("Registered FS handler handler= %x buffer at %x max len %u .. proc %d\n", fs_handler, event_buffer, buffer_max_len, proc->pid);

    save_curr_proc_state(tf, pc);
    proc->status = PROC_NOT_RUNNABLE; // Keep the process waiting for messages
    sched_yield(); // Do not go back to proc rn
    //SET_SYSCALL_RET0(tf, 0)
}




void init_syscalls_fs(void){
    register_syscall(SYS_FS_TOUCH, syscall_touch);
    register_syscall(SYS_FS_RM, syscall_remove);
    register_syscall(SYS_FS_STAT, syscall_stat);
    register_syscall(SYS_FS_REG_HANDLER, syscall_register_handler);
    register_syscall(SYS_FS_RET, syscall_handle_ret);
    
}