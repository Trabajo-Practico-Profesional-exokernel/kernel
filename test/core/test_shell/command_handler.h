#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

// #include "types.h"

typedef int (*command_handler_t)(char *args);

struct CommandEntry {
    char * action_name;
    command_handler_t handler;
};

int exec_test_command(char * action, char* args);
//int send(char* content);
int test_help_man(char * args);

#endif
