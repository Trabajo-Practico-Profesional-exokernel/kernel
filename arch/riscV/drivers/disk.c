

#include "arch/arch_init.h"
#include "arch/trap_handling.h"
#include "inc/common.h"
#include "std/string.h"


void syscall_disk_read(FullTrapFrame *tf, uintptr_t pc) {
    paddr_t src_disk_addr = SYSCALL_ARG0(tf);
    vaddr_t trg_buffer_vaddr = SYSCALL_ARG1(tf);
    size_t read_len = SYSCALL_ARG2(tf);
	
	printf("Got read disk pos: %u to buffer at %x len: %u \n", src_disk_addr, 
            trg_buffer_vaddr, read_len);	


}

void syscall_disk_write(FullTrapFrame *tf, uintptr_t pc) {
    vaddr_t src_buffer_vaddr = SYSCALL_ARG0(tf);
    paddr_t trg_disk_addr = SYSCALL_ARG1(tf);
    size_t write_len = SYSCALL_ARG2(tf);
	
	printf("Got write buffer at %x to buffer pos: %u len: %u \n", 
        src_buffer_vaddr, trg_disk_addr, write_len);	
}

void init_disk(void){
    printf("INITING DISK DRIVER\n");


    printf("INITING RISCV SYSCALLS READ/WRITE\n");

    
    register_syscall(SYS_DISK_READ, syscall_disk_read);
    register_syscall(SYS_DISK_WRITE, syscall_disk_write);

}
