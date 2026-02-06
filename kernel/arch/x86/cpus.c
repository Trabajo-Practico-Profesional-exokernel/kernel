#include "arch/cpus.h"

int cpuid()
{
	return cpunum();
}

int cpuidx()
{
	// apicid and idx are not guaranteed to be the same
	int apicid = cpunum();
	int i = 0;
	for (; i < NCPU; i++) {
		if (cpus[i].cpu_id == apicid)
			break;
	}
    return i;	
}

struct cpu* 
mycpu(void)
{
  int idx = cpuidx();
  return &cpus[idx];
}

struct cpu* 
getcpu(int cpuidx)
{
  return &cpus[cpuidx];  
}

