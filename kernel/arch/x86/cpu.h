#ifndef CPU_H
#define CPU_H

#include "types.h"
#include "tss.h"

enum {
	CPU_UNUSED = 0,
	CPU_STARTED,
	CPU_HALTED,
};


// Per-CPU state ----------> for now ONE CPU
// TODO: for multiple cpus, put gdt inside CpuInfo.
struct CpuInfo {
	uint8_t cpu_id;                 // Local APIC ID; index into cpus[] below
	volatile uint8_t cpu_status;   // The status of the CPU
	// struct Proc *cpu_proc;            // The currently-running environment.
	struct TaskState cpu_ts;        // Used by x86 to find stack for interrupt
};

#define NCPU 8
extern struct CpuInfo cpus[NCPU];

#define cpu (&cpus[0]) // TODO: definir cpunum()

void lapic_init(void);
void lapic_eoi(void);
void lapic_startap(uint8_t apicid, uint32_t addr);

void ioapic_init(void);


#endif
