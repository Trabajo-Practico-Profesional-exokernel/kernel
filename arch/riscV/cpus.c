#include "arch_inc/trap_constants.h"
#include "arch/cpus.h"
#include "inc/common.h"


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


void init_cpus(void){

    // printf("Got cpu id %d \n", get_cpu_id());
    // int id = r_mhartid();
    // set_cpuid(id);
}

