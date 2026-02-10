#ifndef ARCH_INC_CPU
#define ARCH_INC_CPU

#include "types.h"
#include "tss.h"

enum {
	CPU_UNUSED = 0,
	CPU_STARTED,
	CPU_HALTED,
};


struct cpu {
	struct Proc *proc;
	int noff;
	int intena;
	
	volatile unsigned cpu_status;   // The status of the CPU

	// struct Proc *cpu_proc;            // The currently-running environment.
	uint8_t cpu_id;                 // index into cpus[] 
	uint8_t cpu_apicid;             // Local APIC ID; index into cpus[] below
	struct TaskState cpu_ts;        // Used by x86 to find stack for interrupt
};

#define NCPU 8
extern struct cpu cpus[NCPU];

int cpunum(void);
void pic_init(void);

extern paddr_t lapicaddr;
void lapic_init(void);
void lapic_eoi(void);
void lapic_startap(uint8_t apicid, uint32_t addr);

void ioapic_init(void);



#endif /* !*/
