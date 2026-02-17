#include "arch/arch_init.h"
#include "arch/trap_handling.h"
#include "arch/mem.h"
#include "arch/proc.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "console/debug.h"
#include "ide.h"

#define SECTOR_SIZE 512 
#define B2SEC(bsz) (((bsz) + SECTOR_SIZE - 1) / SECTOR_SIZE)

extern void sched_yield(void);
extern void save_curr_proc_state(FullTrapFrame *tf, uintptr_t pc);
extern struct Proc * get_curr(void);

int user_fs_init_sector; 


void 
syscall_disk_read(FullTrapFrame *tf, uintptr_t pc)
{
    debug_printf("Got read disk\n");
	uint32_t sector = SYSCALL_ARG0(tf) + user_fs_init_sector;
	vaddr_t buf_vaddr = SYSCALL_ARG1(tf);
	size_t sz = SYSCALL_ARG2(tf);

	if (sz % SECTOR_SIZE) {
        debug_printf("Invalid size to read: %d", sz);
        SET_SYSCALL_RET0(tf, -1);
        save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
        sched_yield();
		return;
	}
	size_t nsecs = sz / SECTOR_SIZE;

	switch_to_kernel_tables();
	struct Proc *caller = myproc();
	paddr_t buf_paddr = get_paddr_for((uint32_t *) caller->pde_paddr, buf_vaddr);

	if (buf_paddr == 0) {
        debug_printf("Failed invalid vaddr for name = %x not mapped for proc %d\n", buf_vaddr, caller->pid);
        SET_SYSCALL_RET0(tf, -1);
        save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
        sched_yield();
		return;
	}

    debug_printf("Got read disk sector: %u to buffer at %x len: %u from proc %u\n",
			sector,
            buf_paddr, 
			sz, 
			caller->pid
	);

	int r = ide_read((void *)buf_paddr, sector, nsecs);
	SET_SYSCALL_RET0(tf, !r ? sz : -1);
	save_curr_proc_state(tf, pc + 4);
	sched_yield();
	return;
}

// 104021
void
syscall_disk_write(FullTrapFrame *tf, uintptr_t pc)
{
    debug_printf("Got write disk\n");

	vaddr_t buf_vaddr = SYSCALL_ARG0(tf);
	uint32_t sector = SYSCALL_ARG1(tf) + user_fs_init_sector;
	size_t sz = SYSCALL_ARG2(tf);

	if (sz % SECTOR_SIZE) {
        debug_printf("Invalid size to read: %d", sz);
        SET_SYSCALL_RET0(tf, -1);
        save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
        sched_yield();
		return;
	}
	size_t nsecs = sz / SECTOR_SIZE;

	switch_to_kernel_tables();
	struct Proc *caller = myproc();
	paddr_t buf_paddr = get_paddr_for((uint32_t *) caller->pde_paddr, buf_vaddr);
	
	if (buf_paddr == 0) {
        debug_printf("Failed invalid vaddr for name = %x not mapped for proc %d\n", buf_vaddr, caller->pid);
        SET_SYSCALL_RET0(tf, -1);
        save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
        sched_yield();
		return;
	}
    debug_printf("Got write buffer at %x to disk sector: %u len: %u \n", buf_paddr, sector, sz);

	int w = ide_write((void *)buf_paddr, sector, nsecs);
	SET_SYSCALL_RET0(tf, !w ? sz : -1);
	save_curr_proc_state(tf, pc + 4);
	sched_yield();
	return;
}


int read_disk(void *buf, int offset, int length)
{
    int remaining = length;
    int sector = offset / SECTOR_SIZE;
    int sector_offset = offset % SECTOR_SIZE;
    char *dst = (char *)buf;
	char tmp[SECTOR_SIZE];	

    while (remaining > 0) {
		int err = ide_read((void *)tmp, sector, 1);

        if (err != 0) {
            return remaining;   // or return -1;
        }

        int to_copy = SECTOR_SIZE - sector_offset;
        if (to_copy > remaining)
            to_copy = remaining;

        memcpy(dst,
               &tmp[sector_offset],
               to_copy);

        dst += to_copy;
        remaining -= to_copy;

        sector++;
        sector_offset = 0;  // only first sector has an offset
    }
    return 0;
}

void set_user_fs_start(int bytes_offset){
    user_fs_init_sector = (bytes_offset / SECTOR_SIZE) + 1;
    // +1 to ensure its aligned/safe with sector size  
}

void
init_disk(void)
{
	ide_init();
	register_syscall(SYS_DISK_READ, syscall_disk_read);
	register_syscall(SYS_DISK_WRITE, syscall_disk_write);
}

