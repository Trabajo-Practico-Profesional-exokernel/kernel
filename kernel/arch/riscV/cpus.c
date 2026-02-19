#include "arch_inc/trap_constants.h"
#include "arch/cpus.h"
 


struct cpu cpus[NCPU];
int main_cpuid = -1;

int cpuid() {
  return get_cpu_id();
}


struct cpu* mycpu(){
  return &cpus[get_cpu_id()];
}

struct cpu* getcpu(int cpuid){
  return &cpus[cpuid];  
}


void init_cpu_info(void){

    // printf("Got cpu id %d \n", get_cpu_id());
    // int id = r_mhartid();
    // set_cpuid(id);
}

void set_as_main_cpu(void){
  main_cpuid = get_cpu_id();
}

bool is_main_cpu(void){
  return main_cpuid == get_cpu_id();
}

