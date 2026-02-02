#include "interactive_test_interpreter.h"
#include "testing.h"

#include "constants.h"
#include "sched.h"
#include "arch/trap.h"
#include "arch/stdio.h"
#include "arch/arch_init.h"
#include "arch/mem.h"
#include "arch/cpus.h"
#include "arch/trap_handling.h"
// #include "fd.h"
#include "stdio.h"
#include "console/debug.h"
#include "arch_inc/trap_constants.h"

void resume_interactive_shell(void){
	printf("Go back to test shell cpu: %d \n", cpuid());
    interactive_shell_main();

}
void init_interactive_tests(void){
	printf("INITING INTERACTIVE TESTS ON MAIN CPU %d \n", cpuid());

    interactive_shell_help();
    printf("\n[TEST] interactive test shell ready\n");

    interactive_shell_main();

}
void add_interactive_test_core(void){
	printf("ADD NEW INTERACTIVE TEST CORE %d \n", cpuid());
}
