#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

// #include "inc/types.h"

typedef int (*command_handler_t)(char *args);

struct CommandEntry {
    char * action_name;
    command_handler_t handler;
};

int exec_command(char * action, char* args);
int send(char* content);

#endif
