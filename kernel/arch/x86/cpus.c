#include "arch/cpus.h"
#include "arch_inc/mem_constants.h"

extern void init_arch_others(void);
extern pd_entry *kernel_pde;

int cpuid()
{
	return cpunum();
}

int cpuidx()
{
	// apicid and idx are not guaranteed to be the same
	int apicid = cpunum();
	int i = 0;
	for (; i < NCPU; i++) {
		if (cpus[i].cpu_id == apicid)
			break;
	}
    return i;	
}

struct cpu* 
mycpu(void)
{
  int idx = cpuidx();
  return &cpus[idx];
}

struct cpu* 
getcpu(int cpuidx)
{
  return &cpus[cpuidx];  
}


void
start_cpus()
{
	extern uint8_t __bin_entryother[], __bin_entryother_size;
	char *stack;
	
	uint8_t *code = (paddr_t) (0x7000);
	//memmove(code, __bin_entryother, __bin_entryother_size);

	for (struct cpu *c = cpus; c->cpu_id < NCPU; c++) {
		if (c == mycpu() || c->cpu_status == CPU_UNUSED)
			continue;

		stack = alloc_pages(1);
		*(void **)(code-4) = stack + KSTKSIZE;
		*(void **)(code-8) = init_arch_others;
		*(void **)(code-12) = kernel_pde;

		lapic_startap(c->cpu_id, code);

		while (c->cpu_status == CPU_STARTED);
	}
}

