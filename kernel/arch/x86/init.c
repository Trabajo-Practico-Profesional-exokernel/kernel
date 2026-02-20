#include "stdio.h"
#include "arch/arch_init.h"
#include "arch/mem.h"
#include "arch/cpus.h"
#include "types.h"
 
#include "arch_inc/cpu.h"
#include "arch_inc/x86.h"
#include "arch_inc/mem_constants.h"
#include "idt.h"
#include "gdt.h"
#include "interrupt.h"

#include "serial_handler.h"

//struct CpuInfo cpus[NCPU];
#define GD_KD  0x10

uint32_t percpu_kstacks[NCPU];
__attribute__((__aligned__(PAGE_SIZE)));

extern char __trap_stack_top[];
extern pd_entry *kernel_pde;
extern idt_gate_t idt[IDT_NUM_ENTRIES];
extern void secondary_cpu_main(void);

static void tss_init(void){
    struct TaskState ts = { .prev_tss = 0, .esp0 = __trap_stack_top, .ss0 = GD_KD };
	mycpu()->cpu_apicid = lapic_id();
	mycpu()->cpu_status = CPU_STARTED;

    // TODO: hacerlo mas lindo (e investigar)
}


void
kstack_alloc()
{
	for (int i = 0; i < NCPU; i++) {
		// alloc KSTKSIZE + 1 guard page
		percpu_kstacks[i] = alloc_pages(KSTKSIZE/PAGE_SIZE + 1);
		if (percpu_kstacks[i] == NULL)
			PANIC("kernel stack allocation failed");
	}
}


void
init_arch(void)
{
	kstack_alloc();
	init_cpus();
    disable_interrupts();
    gdt_init();
    idt_init();
	lapic_init();
	pic_init();
	ioapic_init();
    serial_init();
	//enable_interrupts();
}

void
init_arch_others(void)
{
	switch_to_kernel_tables(); // redundant; its done in entryother.S
	gdt_init();
	lidt(idt, sizeof(idt));
	lapic_init();
	mycpu()->cpu_status = CPU_STARTED;
	xchg(&mycpu()->cpu_status, CPU_STARTED); // tell start_cpus() we're up

	printf("CPU %d now RUNNING\n", mycpu()->cpu_id);
	while(1);
	secondary_cpu_main();
}


