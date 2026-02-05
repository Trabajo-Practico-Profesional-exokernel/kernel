#include "string.h"
#include "stdlib.h"
#include "console/debug.h"
#include "parsers/strutil.h"
#include "arch/stdio.h"

// Own includes
#include "test_shell/command_handler.h"
#include "test_shell/utils.h"
#include "test_shell/interactive_info_logging.h"
#include "interactive_test_interpreter.h"
#include "sched.h"
#include "arch/cpus.h"
#include "arch_inc/trap_constants.h"



bool enabled_clock_yield_logging = false;
bool enabled_clock_yield_interactive = false;

int enable_clock_yield_logging(char* args){
	enabled_clock_yield_logging = true;
	return 0;
}
int disable_clock_yield_logging(void){
	enabled_clock_yield_logging = false;

	return 0;
}

int enable_clock_yield_interactive(char* args){

	enabled_clock_yield_interactive = true;
	return 0;

}
int disable_clock_yield_interactive(void){
	enabled_clock_yield_interactive= false;
	return 0;
}


// Define some logic to do on clock yield?
// You could check idle time, which process is current and so on!
// You could log info for a test in python or external to parse and eval!
void on_clock_yield(uint32_t new_curr_slices){
	if(enabled_clock_yield_interactive){
	    disable_timer_interrupts();
	    interactive_shell_main();
	    return;
	}
	if(enabled_clock_yield_logging){
	    printf("[TEST TICK] idle time slice tot idle: %u tot ticks = %u\n",get_idle_ticks(), get_real_ticks());
	}	
}
