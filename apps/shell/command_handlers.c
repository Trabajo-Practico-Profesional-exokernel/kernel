#include "lib.h"
#include "std/string.h"

#include "inc/types.h"
#include "std/printf.h"
#include "default_executables.h"

#include "command_handler.h"


// Just one arg? the progam to exec.. maybe also the args for it .. not for now? 
int start_program(char* program_name){
    char * args = NULL;
    split_by_once(program_name, &args, ' ');
    return exec_program(program_name, args);

}

int handle_exec(char* args){
    int pid_child = start_program(args);

    if (pid_child == -1){ // lets assume a proc cannot hav -1 as pid? maybe for now this works lol
        return ERR_CODE;
    }
    // Wait for the child!
    int ret_code= wait(pid_child);
    printf("Waited for proc %d! exited with code %d\n", pid_child, ret_code);

    return ret_code;
}

int handle_wait(char* args){

    long pid_waited = strtol(args, NULL, 0);
    int ret_code= 0;
    // int ret_code= wait(pid_waited);
    printf("Waited for proc %d! exited with code %d\n", pid_waited, ret_code);

    return ret_code;
    
}


struct CommandEntry commands[] = {
    { "exec",  handle_exec},
    { "start",  start_program},
    { "msg", send},
    { "wait", handle_wait}
};

#define COMMAND_COUNT 4

int send(char* content){
    
    char* msg_out;

    int pid_proc = parse_num_and_msg(content, &msg_out);
    if (pid_proc == -1){
        return ERR_CODE;
    }

    int success = try_send_msg(pid_proc, content, strlen(content));
    if (success) {
        printf("MSG [%s] sended to process with PID [%u] \n", content, pid_proc);
    } else {
        printf("Failed sending MSG [%s] to process with PID [%u] \n", content, pid_proc);
    }
    return OK_CODE;
}


int exec_command(char * action, char* args){
    int len_act = strlen(action);
    for (int i = 0; i < COMMAND_COUNT; i++) {
        if (strncmp(action, commands[i].action_name, len_act) == 0) {
            return commands[i].handler(args);
        }
    }
    int ret_code = ERR_CODE;

    if (default_executable_check(action, args, &ret_code)){
        return ret_code;
    }
    
    printf("\nUnknown Command: '%s' args '%s'\n", action, args);
    return ret_code;
}