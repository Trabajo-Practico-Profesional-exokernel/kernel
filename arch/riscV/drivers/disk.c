

#include "arch/arch_init.h"
#include "arch/trap_handling.h"
#include "arch/mem.h"

#include "inc/common.h"
#include "std/string.h"

#include "arch_inc/virtio_blk.h"
#include "arch_inc/virtio.h"

extern void sched_yield(void);
extern void save_curr_proc_state(FullTrapFrame *tf, uintptr_t pc);

void syscall_disk_read(FullTrapFrame *tf, uintptr_t pc) {
    paddr_t src_disk_addr = SYSCALL_ARG0(tf);
    vaddr_t trg_buffer_vaddr = SYSCALL_ARG1(tf);
    size_t read_len = SYSCALL_ARG2(tf);
	
	printf("Got read disk pos: %u to buffer at %x len: %u \n", src_disk_addr, 
            trg_buffer_vaddr, read_len);	

    switch_to_kernel_tables();

    char buf[SECTOR_SIZE];
    read_write_disk(buf, 0, false /* read from the disk */);
    printf("first sector: %s\n", buf);
    save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
    
    sched_yield();
}

void syscall_disk_write(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t src_buffer_vaddr = SYSCALL_ARG0(tf);
    paddr_t trg_disk_addr = SYSCALL_ARG1(tf);
    size_t write_len = SYSCALL_ARG2(tf);
    // switch_to_kernel_tables();
	
	printf("Got write buffer at %x to buffer pos: %u len: %u \n", 
        src_buffer_vaddr, trg_disk_addr, write_len);	

    switch_to_kernel_tables();

    char buf[SECTOR_SIZE];
    strcpy(buf, "hello from kernel!!!\n");
    read_write_disk(buf, 0, true /* write to the disk */);
    
    save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
    sched_yield();

}

void init_disk(void){
    printf("INITING VIRTIO\n");
    virtio_init();

    printf("INITING VIRTIO BLK data\n");
    virtio_blk_init();


    printf("INITING RISCV SYSCALLS READ/WRITE\n");

    register_syscall(SYS_DISK_READ, syscall_disk_read);
    register_syscall(SYS_DISK_WRITE, syscall_disk_write);

}
