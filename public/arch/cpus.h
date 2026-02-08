#ifndef INC_CPUS
#define INC_CPUS

#include "types.h"
#include "arch_inc/cpu.h"

void init_cpu_info(void);

void set_as_main_cpu(void);
bool is_main_cpu(void);

int cpuid();

struct cpu* mycpu(void);

struct cpu* getcpu(int cpuid);

struct cpu* mycpu();

#endif /* !*/


