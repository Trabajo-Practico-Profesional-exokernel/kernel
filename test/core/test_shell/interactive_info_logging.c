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









int handle_disable_log_irq(char* args) {
    return disable_clock_yield_logging();    
}
int handle_disable_interactive_irq(char* args) {
    return disable_clock_yield_interactive();    
}


void init_irq_commands(void){
    add_test_command((struct CommandEntry){
        .action_name = "irq_log_on",
        .handler = enable_clock_yield_logging,
        .description = "habilita logs de informacion en cada interrupcion por clock"
    });

    add_test_command((struct CommandEntry){
        .action_name = "irq_log_off",
        .handler = handle_disable_log_irq,
        .description = "deshabilita logs de informacion en cada interrupcion por clock"
    });

    add_test_command((struct CommandEntry){
        .action_name = "irq_to_shell_on",
        .handler = enable_clock_yield_interactive,
        .description = "habilita que en cada interrupcion por clock se vuelva a la test shell"
    });

    add_test_command((struct CommandEntry){
        .action_name = "irq_to_shell_off",
        .handler = handle_disable_interactive_irq,
        .description = "deshabilita que en cada interrupcion por clock se vuelva a la test shell"
    });

}