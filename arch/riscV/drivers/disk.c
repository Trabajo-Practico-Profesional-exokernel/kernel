

#include "arch/arch_init.h"
#include "arch/trap_handling.h"
#include "arch/mem.h"
#include "arch/proc.h"

 
#include "std/string.h"

#include "arch_inc/virtio_blk.h"
#include "arch_inc/virtio.h"

extern void sched_yield(void);
extern void save_curr_proc_state(FullTrapFrame *tf, uintptr_t pc);
extern struct Proc * get_curr(void);

char disk_request_content_buffer[SECTOR_SIZE];

int check_valid_size_read(size_t len){
    if (len > SECTOR_SIZE){
        debug_printf("Error content len was too long %u, max allowed is %u \n", len, SECTOR_SIZE);
        return -2;
    }

    // if (len < SECTOR_SIZE){
    //     debug_printf("Error content len was less than buffer size %u < %u, for now not allowed\n", len, SECTOR_SIZE);
    //     return -3;
    // }

    return 0;
}

int check_valid_size_write(size_t len){
    if (len > SECTOR_SIZE){
        debug_printf("Error content len was too long %u, max allowed is %u \n", len, SECTOR_SIZE);
        return -2;
    }

    if (len < SECTOR_SIZE){
        debug_printf("Error content len was less than buffer size %u < %u, for now not allowed\n", len, SECTOR_SIZE);
        return -3;
    }

    return 0;
}

void syscall_disk_read(FullTrapFrame *tf, uintptr_t pc) {
    
    virt_blk_sector_t src_disk_sector = SYSCALL_ARG0(tf);
    vaddr_t trg_buffer_vaddr = SYSCALL_ARG1(tf);
    size_t read_len = SYSCALL_ARG2(tf);

    int err = check_valid_size_read(read_len);
	if(err < 0){
        SET_SYSCALL_RET0(tf, err);
        return;
    }
    switch_to_kernel_tables();
    
    struct Proc * caller = get_curr();

    // Map vaddr to paddr
    paddr_t trg_buffer_paddr = get_paddr_for(
                                    (uint32_t *) caller->pde_paddr,
                                    trg_buffer_vaddr);

    if (trg_buffer_paddr == 0){
        debug_printf("Failed invalid vaddr for name = %x not mapped for proc %d\n", trg_buffer_vaddr, caller->pid);
        SET_SYSCALL_RET0(tf, -1);
        save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
        sched_yield();
        return;
    }


	debug_printf("Got read disk sector: %u to buffer at %x len: %u from proc %u\n", src_disk_sector, 
            trg_buffer_paddr, read_len, caller->pid);	


    err = read_write_disk(&disk_request_content_buffer[0],
        src_disk_sector, false /* read from the disk */);
    if(err < 0){
        SET_SYSCALL_RET0(tf, err);
        save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
        sched_yield();
        return;
    }

    debug_printf("first sector: '%s'\n", &disk_request_content_buffer[0]);

    memcpy((char*)trg_buffer_paddr, &disk_request_content_buffer[0], read_len);
    
    SET_SYSCALL_RET0(tf, read_len);
    save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
    sched_yield();
}

void syscall_disk_write(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t src_buffer_vaddr = SYSCALL_ARG0(tf);
    paddr_t trg_disk_sector = SYSCALL_ARG1(tf);
    size_t write_len = SYSCALL_ARG2(tf);

    int err = check_valid_size_write(write_len);
    if(err < 0){
        SET_SYSCALL_RET0(tf, err);
        return;
    }

    switch_to_kernel_tables();

    struct Proc * caller = get_curr();

    // Map vaddr to paddr
    paddr_t src_buffer_paddr = get_paddr_for(
                                    (uint32_t *) caller->pde_paddr,
                                    src_buffer_vaddr);

    if (src_buffer_paddr == 0){
        debug_printf("Failed invalid vaddr for name = %x not mapped for proc %d\n", src_buffer_vaddr, caller->pid);
        SET_SYSCALL_RET0(tf, -1);
        save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
        sched_yield();
        return;
    }

	debug_printf("Got write buffer at %x to disk sector: %u len: %u \n", 
        src_buffer_paddr, trg_disk_sector, write_len);	

    memcpy(&disk_request_content_buffer[0], (char*)src_buffer_paddr, write_len);

    err = read_write_disk(&disk_request_content_buffer[0], trg_disk_sector, true /* write to the disk */);
    
    if(err < 0){
        SET_SYSCALL_RET0(tf, err);
        save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
        sched_yield();
        return;
    }
    SET_SYSCALL_RET0(tf, 0);

    save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
    sched_yield();
}

void init_disk(void){
    debug_printf("INITING VIRTIO\n");
    virtio_init();

    debug_printf("INITING VIRTIO BLK data\n");
    virtio_blk_init();


    debug_printf("INITING RISCV SYSCALLS READ/WRITE\n");

    register_syscall(SYS_DISK_READ, syscall_disk_read);
    register_syscall(SYS_DISK_WRITE, syscall_disk_write);

}
