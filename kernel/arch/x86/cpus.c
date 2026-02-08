#include "arch/cpus.h"
#include "arch_inc/trap_constants.h"
 

struct cpu cpus_structs[NCPU];

int cpuid() {
  return get_cpu_id();
}

struct cpu* mycpu(void){
  return &cpus_structs[cpuid()];
}

struct cpu* getcpu(int cpuid){
  return &cpus_structs[cpuid];  
}


void init_cpu_info(void){

    // printf("Got cpu id %d \n", get_cpu_id());
    // int id = r_mhartid();
    // set_cpuid(id);
}

