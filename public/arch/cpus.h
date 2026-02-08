#ifndef INC_CPUS
#define INC_CPUS

#include "types.h"
#include "arch_inc/cpu.h"

void init_cpus(void);

void start_cpus(void);

int cpuid();

struct cpu* mycpu(void);

struct cpu* getcpu(int cpuid);

#endif /* !*/


