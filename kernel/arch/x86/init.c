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

extern char __trap_stack_top[];
extern pd_entry *kernel_pde;
extern struct Segdesc gdt[GDT_NUM_ENTRIES];
extern idt_gate_t idt[IDT_NUM_ENTRIES];
extern void secondary_cpu_main(void);

static void cpu_init(void){
    struct TaskState ts = { .prev_tss = 0, .esp0 = __trap_stack_top, .ss0 = GD_KD };
	mycpu()->cpu_apicid = lapic_id();
	mycpu()->cpu_status = CPU_STARTED;

    // TODO: hacerlo mas lindo (e investigar)
    ts.cs = 0x0b;
	ts.ss = 0x13;
	ts.es = 0x13;
	ts.ds = 0x13;
	ts.fs = 0x13;
	ts.gs = 0x13;

    mycpu()->cpu_ts = ts;
}

void
init_arch(void)
{
	init_cpus();
    disable_interrupts();
    gdt_init();
    idt_init();
	lapic_init();
	pic_init();
	ioapic_init();
    serial_init();
    cpu_init();
	enable_interrupts();
}

void
init_arch_others(void)
{
	switch_to_kernel_tables(); // redundant; its done in entryother.S
	lgdt(gdt, sizeof(gdt));
	lapic_init();
	lidt(idt, sizeof(idt));
	//cpu_init();
	mycpu()->cpu_status = CPU_STARTED;
	xchg(&mycpu()->cpu_status, CPU_STARTED); // tell start_cpus() we're up

	printf("CPU %d now RUNNING\n", mycpu()->cpu_id);
	while(1);
	secondary_cpu_main();
}

