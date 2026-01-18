#ifndef INC_CPUS
#define INC_CPUS

#include "inc/types.h"
#include "arch_inc/cpu.h"

void init_cpus(void);

int cpuid();

struct cpu* mycpu(void);

struct cpu* getcpu(int cpuid);

#endif /* !*/


