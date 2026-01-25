#include "arch/arch_init.h"
#include "arch/trap_handling.h"
#include "arch/mem.h"
#include "arch/proc.h"
#include "std/printf.h"
#include "std/string.h"
#include "console/debug.h"
#include "ide.h"


extern void sched_yield(void);
extern void save_curr_proc_state(FullTrapFrame *tf, uintptr_t pc);
extern struct Proc * get_curr(void);


void 
syscall_disk_read(FullTrapFrame *tf, uintptr_t pc)
{
    debug_printf("Got read disk\n");
	uint32_t sector = SYSCALL_ARG0(tf);
	vaddr_t buf_vaddr = SYSCALL_ARG1(tf);
	size_t sz = SYSCALL_ARG2(tf);

	switch_to_kernel_tables();
	struct Proc *caller = get_curr();
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

	int r = ide_read((void *)buf_paddr, sector, sz);
	SET_SYSCALL_RET0(tf, sz);
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
	uint32_t sector = SYSCALL_ARG1(tf);
	size_t sz = SYSCALL_ARG2(tf);

	switch_to_kernel_tables();
	struct Proc *caller = get_curr();
	paddr_t buf_paddr = get_paddr_for((uint32_t *) caller->pde_paddr, buf_vaddr);
	
	if (buf_paddr == 0) {
        debug_printf("Failed invalid vaddr for name = %x not mapped for proc %d\n", buf_vaddr, caller->pid);
        SET_SYSCALL_RET0(tf, -1);
        save_curr_proc_state(tf, pc + 4); // Skip this ins that called syscall
        sched_yield();
		return;
	}
    debug_printf("Got write buffer at %x to disk sector: %u len: %u \n", buf_paddr, sector, sz);

	int w = ide_write((void *)buf_paddr, sector, sz);
	SET_SYSCALL_RET0(tf, w);
	save_curr_proc_state(tf, pc + 4);
	sched_yield();
	return;
}

void
init_disk(void)
{
	ide_init();
	register_syscall(SYS_DISK_READ, syscall_disk_read);
	register_syscall(SYS_DISK_WRITE, syscall_disk_write);
}

