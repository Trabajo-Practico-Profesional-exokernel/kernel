#include "parsers/strutil.h"
#include "console/debug.h"
#include "string.h"
#include "stdlib.h"
#include "test_shell/command_handler.h"
#include "interactive_test_commands.h"
#include "test_shell/interactive_info_logging.h"

int test_help_man(char* args);

#define MAX_COMMAND_COUNT  128


int curr_command_count =1;

struct CommandEntry commands[MAX_COMMAND_COUNT] = {
    { "help", test_help_man, "Da informacion general de comandos"}
};

void add_test_command(struct CommandEntry command){
    if(curr_command_count >=MAX_COMMAND_COUNT){
        PANIC("Failed add test command shell, overflow!\n");
    }
    commands[curr_command_count] = command;
    curr_command_count+=1;
    

}

// Auto-calculate command count
int test_help_man(char* args) {
    printf("Podes correr:\n");
    for (unsigned int i = 0; i < curr_command_count; i++) {
        printf("'%s' %s\n",commands[i].action_name, commands[i].description);
    }
    return 0;
}


int exec_test_command(char * action, char* args){
    int len_act = strlen((const uint8_t*)action) + 1;// include 0 byte

    for (unsigned int i = 0; i < curr_command_count; i++) {

        char* trg_action = commands[i].action_name;
        if (strncmp((const uint8_t*)action, (const uint8_t*)trg_action, len_act) == 0) {
            if(!args){ // Fill with empty if not defined.
                args = "";
            }
            return commands[i].handler(args);
        }
    }
    printf("\nUnknown Test Command: '%s' args '%s'\nRun 'help' for information about commands\n", action, args);
    return -1;
}
