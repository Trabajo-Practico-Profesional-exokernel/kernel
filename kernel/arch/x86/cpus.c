#include "constants.h"
#include "arch/cpus.h"
#include "arch_inc/mem_constants.h"

extern void init_arch_others(void);
extern pd_entry *kernel_pde;

int cpuid(void)
{
	return mycpu()->cpu_id;
}

struct cpu* 
mycpu(void)
{
	// apicid and idx are not guaranteed to be the same
	int apicid = lapic_id();
	int i = 0;
	for (; i < NCPU; i++) {
		if (cpus[i].cpu_apicid == apicid)
			return &cpus[i];
	}
	PANIC("SMP: unknown apicid: %d\n; corresponding apicid: %d", apicid, cpus[apicid].cpu_apicid);
}

struct cpu* 
getcpu(int cpuidx)
{
  return &cpus[cpuidx];  
}


void *cpu_stack;

void
start_cpus(void)
{
	extern uint8_t entryother_start[], entryother_end[]; // defined in entryother.s
	//char *stack;
	
	uint8_t *code = (paddr_t) (0x7000);
	memmove(code, entryother_start, entryother_end - entryother_start);

	for (struct cpu *c = cpus; c - cpus < NCPU; c++) {
		if (c == mycpu() || c->cpu_status == CPU_UNUSED)
			continue;

		cpu_stack = alloc_pages(1) + KSTKSIZE;
    	//*(uint32_t **)(code + 4) = kernel_pde;
    	//*(uint32_t **)(code + 8) = init_arch_others;
    	//*(uint32_t **)(code + 12) = stack + KSTKSIZE;

		lapic_startap(c->cpu_apicid, code);

		while (c->cpu_status != CPU_STARTED);
	}
}

