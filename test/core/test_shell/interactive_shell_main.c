#include "string.h"
#include "stdlib.h"
#include "console/debug.h"
#include "parsers/strutil.h"
#include "console/debug.h"

// Own includes
#include "test_shell/command_handler.h"
#include "test_shell/utils.h"
#include "interactive_test_interpreter.h"
#include "arch/stdio.h"
#include "sched.h"
#include "arch/cpus.h"

#define SLEEP_TIME 300000000

#define MAX_INPUT 128
char input_buf[MAX_INPUT];

char* running_command = NULL;

void interactive_shell_help(void){
    test_help_man("");    

}

void resume_interactive_shell(struct Proc * last_proc){
    if(last_proc && last_proc->status == PROC_NOT_RUNNABLE){
        switch_to_idle_proc();        
    }
    printf("Go back to test shell cpu: %d \n", cpuid());
    if(running_command){
        printf("\n[TEST] interactive test shell command executed:\n'%s'\n",running_command);
        running_command = NULL;
    }

    interactive_shell_main();
}


void interactive_shell_main(void) {
    static char input_buf[MAX_INPUT];
    running_command = NULL;
    while (1){
        printf("\n[tester command]>\n");
        // int len = 
        read_line(input_buf, MAX_INPUT);

        char * args= NULL;
        split_by_once((uint8_t*)input_buf, (uint8_t**)&args, ' ');


        if (strncmp((const uint8_t*)input_buf, (const uint8_t*)"q", 2) == 0) {
            printf("\n[TEST] interactive test shell exited\n");
            break;
        }
        printf("\n");
        running_command = input_buf;
        exec_test_command(input_buf, args);
        running_command = NULL;
        printf("\n[TEST] interactive test shell command executed:\n'%s'\n",input_buf);
    }
}