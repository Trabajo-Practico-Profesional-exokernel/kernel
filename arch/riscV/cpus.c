#include "arch_inc/trap_constants.h"
#include "arch/cpus.h"


struct cpu cpus[NCPU];

int cpuid() {
  return get_cpu_id();
}

struct cpu* mycpu(void){
  return &cpus[cpuid()];
}

struct cpu* getcpu(int cpuid){
  return &cpus[cpuid];  
}


