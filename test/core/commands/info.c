#include "status_logging.h"
#include "arch/proc.h"
#include "parsers/strutil.h"
#include "stdlib.h"
#include "string.h"

#include "test_shell/command_handler.h"
#include "interactive_test_commands.h"
#include "console/debug.h"

char * RUNNING_LABEL = "running";
char * BLOCKED_LABEL = "blocked";
char * RUNNABLE_LABEL = "runnable";
char * DYING_LABEL = "dying";
char * CREATED_LABEL = "created";

int show_info_ls_procs(char * args){
    int len_arg = strlen(args) +1;
    if (len_arg == 1 || strncmp((const uint8_t*)CREATED_LABEL, (const uint8_t*)args, len_arg) == 0) {
        info_procs_not_free();
        return 0;
    }

    if (strncmp((const uint8_t*)RUNNING_LABEL, (const uint8_t*)args, len_arg) == 0) {
        info_procs_with_state(PROC_RUNNING);
        return 0;
    }
    if (strncmp((const uint8_t*)BLOCKED_LABEL, (const uint8_t*)args, len_arg) == 0) {
        info_procs_with_state(PROC_NOT_RUNNABLE);
        return 0;
    }
    if (strncmp((const uint8_t*)RUNNABLE_LABEL, (const uint8_t*)args, len_arg) == 0) {
        info_procs_with_state(PROC_RUNNABLE);
        return 0;
    }

    if (strncmp((const uint8_t*)DYING_LABEL, (const uint8_t*)args, len_arg) == 0) {
        info_procs_with_state(PROC_DYING);
        return 0;
    }

    printf("Invalid state '%s'\n",args);
    return -1;
}
int show_info_proc_exit_status(char * args){

}
int show_info_proc_uptime(char * args){

}
int show_info_proc_memory(char * args){

}
int show_info_uptimes(char * args){

}
int show_info_user_mem(char * args){

}


void init_info_commands(void){
    add_test_command((struct CommandEntry){
        .action_name = "ls_procs",
        .handler = show_info_ls_procs,
        .description = "recibe: <filtro>, lista todos los procesos con tal filtro, disponibles 'running', 'runnable', 'blocked', 'created', 'dying'"
    });
}

