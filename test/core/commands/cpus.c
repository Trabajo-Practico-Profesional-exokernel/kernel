#include "interactive_test_commands.h"
#include "arch/arch_init.h"
#include "arch/spin_locks.h"
#include "arch/cpus.h"
#include "arch_inc/trap_constants.h"
#include "console/debug.h"

struct spinlock cpus_lock;


volatile static int started = 0;

int count_started = 1;

void wait_start_cpus(void){
    while(started == 0)
          ;	
}

void start_cpus(void){
	cpus_lock.name = "STARTED CPUS LOCK";	
	start_secondary_cpus();
	started = 1;
}

int get_curr_count_started(){
    acquire(&cpus_lock);

	int ret = count_started;

    release(&cpus_lock);

    return ret;
}

void wait_cpus_started(){
    acquire(&cpus_lock);
    printf("...cpu %d will wait all cpus start\n", cpuid());
    release(&cpus_lock);

	while(get_curr_count_started() < NCPU)
		;
}

void add_start_cpu(void){
    acquire(&cpus_lock);
	
	count_started+=1;
	printf("Started cpu %d, new count cpus started %d/%d\n",cpuid(), count_started, NCPU);

    release(&cpus_lock);
}
