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
#include "arch_inc/trapframe.h"
#include "proc_sleeping.h"
#include "arch/logging.h"

bool enabled_syscall_interactive = false;
int looked_upon_sysno = -1;
int trg_uptime = -1;
int next_trg_uptime = -1;
bool enabled_clock_yield_interactive = false;

int enable_on_syscall_interactive(char* args){
	enabled_syscall_interactive = true;
	if(strlen(args) > 0){
		looked_upon_sysno = atoi(args);
		printf("Enabled trigger go to test shell on syscall %d\n",looked_upon_sysno);	
	} else {
		looked_upon_sysno = -1;			
		printf("Enabled trigger go to test shell on any syscall\n");	
	}
	
	return 0;
}
int disable_syscall_interactive(void){
	enabled_syscall_interactive = false;

	return 0;
}

int enable_clock_yield_interactive(char* args){

	enabled_clock_yield_interactive = true;
	if(strlen(args) > 0){
		trg_uptime = atoi(args);
		next_trg_uptime = get_sys_uptime() + trg_uptime;
		printf("Enabled trigger go to test shell on clock yield after uptime cycle: %d, next trg: %d\n",trg_uptime, next_trg_uptime);
	} else {
		trg_uptime = -1;			
		next_trg_uptime = 0;
		printf("Enabled trigger go to test shell on next clock yield\n");	
	}


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

		int sys_ticks = get_sys_uptime();
		if(next_trg_uptime > sys_ticks){
			return;
		}

	    disable_timer_interrupts();

	    #ifdef IS_RISC
	    // Backup stuff.
	    uintptr_t scause = READ_CSR(scause);
	    uintptr_t stval = READ_CSR(stval);
	    uintptr_t user_pc = READ_CSR(sepc);	    
	    #endif
	   	
		printf("On clock yield uptime: %u >= %u (target)",sys_ticks, next_trg_uptime);
	   	if(trg_uptime >=0){
		    next_trg_uptime = next_trg_uptime+trg_uptime;
		   	printf(" next trg: %d\n", next_trg_uptime);
	   	} else {
		   	printf("\n");
	   	}

	    interactive_shell_main();
	    #ifdef IS_RISC
	    // RESTORE stuff.
	    WRITE_CSR(scause, scause);
	    WRITE_CSR(stval,stval);
	    WRITE_CSR(sepc, user_pc);	    
	    #endif
	    return;
	}
}

extern void on_syscall_called(unsigned sysno, FullTrapFrame *tf, uintptr_t pc){
	if(enabled_syscall_interactive){
		if(looked_upon_sysno >=0 && looked_upon_sysno != sysno){
			// printf("Not lookedup sysno %u vs wanted %u\n",sysno, looked_upon_sysno);
			return;
		}
	    disable_timer_interrupts();

	    printf("======= BEFORE EXEC SYSCALL\nRunning: ");
	    struct Proc * curr_proc = myproc();
	    info_proc(curr_proc);
	    printf("=== SYSCALL %u PC 0x%x SYSCALL TF: ", sysno,pc);
	    printTrapFull(tf);

	    interactive_shell_main();

	    return;
	}
}





int handle_disable_syscall_interactive(char* args) {
    return disable_syscall_interactive();    
}
int handle_disable_interactive_irq(char* args) {
    return disable_clock_yield_interactive();    
}


void init_irq_commands(void){
    add_test_command((struct CommandEntry){
        .action_name = "sys_to_shell",
        .handler = enable_on_syscall_interactive,
        .description = "habilita que en cada syscall se vuelva a la test shell"
    });

    add_test_command((struct CommandEntry){
        .action_name = "sys_to_shell_off",
        .handler = handle_disable_syscall_interactive,
        .description = "deshabilita que en cada syscall se vuelva a la test shell"
    });

    add_test_command((struct CommandEntry){
        .action_name = "irq_to_shell",
        .handler = enable_clock_yield_interactive,
        .description = "habilita que en cada interrupcion por clock se vuelva a la test shell"
    });

    add_test_command((struct CommandEntry){
        .action_name = "irq_to_shell_off",
        .handler = handle_disable_interactive_irq,
        .description = "deshabilita que en cada interrupcion por clock se vuelva a la test shell"
    });

}