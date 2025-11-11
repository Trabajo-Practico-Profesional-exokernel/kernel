#include "types.h"
#include "tss.h"

enum {
	CPU_UNUSED = 0,
	CPU_STARTED,
	CPU_HALTED,
};
#define NCPU 8
struct CpuInfo cpus[NCPU];

#define cpu (&cpus[0]) // TODO: definir cpunum()


// Per-CPU state ----------> for now ONE CPU
struct CpuInfo {
	uint8_t cpu_id;                 // Local APIC ID; index into cpus[] below
	volatile unsigned cpu_status;   // The status of the CPU
	// struct Proc *cpu_proc;            // The currently-running environment.
	struct TaskState cpu_ts;        // Used by x86 to find stack for interrupt
};