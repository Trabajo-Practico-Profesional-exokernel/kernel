#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

// #include "types.h"

typedef int (*command_handler_t)(int argc, char **argv);

struct CommandEntry {
    char * action_name;
    command_handler_t handler;
};

command_handler_t find_command_builtin(char * action);
//int send(char* content);

int command_default_exec(char* proc_name, char** argv);

char * get_curr_path(void);
#endif
