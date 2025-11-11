#include "inc/types.h"
#include "tss.h"

enum {
	CPU_UNUSED = 0,
	CPU_STARTED,
	CPU_HALTED,
};
#define NCPU 8

#define cpu (&cpus[0]) // TODO: definir cpunum()


// Per-CPU state ----------> for now ONE CPU
// TODO: for multiple cpus, put gdt inside CpuInfo.
struct CpuInfo {
	uint8_t cpu_id;                 // Local APIC ID; index into cpus[] below
	// struct Proc *cpu_proc;            // The currently-running environment.
	struct TaskState cpu_ts;        // Used by x86 to find stack for interrupt
};

struct CpuInfo cpus[NCPU];

