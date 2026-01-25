#include "lib.h"
#include "std/string.h"
#include "std/printf.h"
#include "app_names.h"
#include "console/debug.h"
#include "parsers/strutil.h"

extern char* _app_names[];

#include "command_handler.h"

// Just one arg? the progam to exec.. maybe also the args for it .. not for now? 
int start_program(char* program_name){
    char * args = NULL;
    split_by_once(program_name, &args, ' ');


    for (int ind_program = 0; ind_program < APP_COUNT; ind_program++) {
        if (strncmp(program_name, _app_names[ind_program] , strlen(program_name)) == 0) {
            debug_printf("SHOULD RUN AT INDEX! %d: '%s' args '%s'\n", ind_program, program_name, args);
            
            int proc_pid= exec(ind_program, &args);

            debug_printf("Program %s started proc_id is ... %d\n", program_name, proc_pid);
            return proc_pid;
        }
    }
    
    debug_printf("At exec '%s' program not recognized\n", program_name);
    return ERR_CODE;
}

int handle_exec(char* args){
    int pid_child = start_program(args);

    if (pid_child == -1){ // lets assume a proc cannot hav -1 as pid? maybe for now this works lol
        return ERR_CODE;
    }
    // Wait for the child!
    debug_printf("Should wait for  proc %d end!\n", pid_child);
    return OK_CODE;
}


struct CommandEntry commands[] = {
    { "exec",  handle_exec},
    { "start",  start_program},
};

#define COMMAND_COUNT 2




int exec_command(char * action, char* args){
    int len_act = strlen(action);
    for (int i = 0; i < COMMAND_COUNT; i++) {
        if (strncmp(action, commands[i].action_name, len_act) == 0) {
            return commands[i].handler(args);
        }
    }        
    debug_printf("\nUnknown Command: '%s' args '%s'\n", action, args);
    return ERR_CODE;
}