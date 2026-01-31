#include "parsers/strutil.h"
#include "console/debug.h"
#include "string.h"
#include "stdlib.h"
#include "test_shell/command_handler.h"
#include "interactive_test_commands.h"

int test_handle_run_shell(char* args) {
    printf("Should run shell with args '%s'\n", args);
    return 0;
}

int test_handle_run_tests(char* args) {
    printf("Should run test with args '%s'\n", args);
    return 0;
}

int test_start_cpus(char* args) {
    printf("Starting secondary cpus\n");
    start_cpus();
    wait_cpus_started();
    printf("...All cpus started\n");

    return 0;
}

int test_help_man(char* args) {
    printf("Podes correr:\n");
    printf("'run_shell <args para la shell>' corre la shell de usuario y podes probar programas de la misma\n");
    printf("'run_tests <args para los tests>' corre uno o mas tests predefinidos del kernel, en testing.c\n");
    printf("'start_cpus' empieza el sistema multicore, levanta/desbloquea los cores\n");
    return 0;
}

struct CommandEntry commands[] = {
    { "run_shell", test_handle_run_shell },
    { "run_tests", test_handle_run_tests },
    { "start_cpus", test_start_cpus },
    { "help", test_help_man }
    
};
// Auto-calculate command count
#define COMMAND_COUNT (sizeof(commands) / sizeof(struct CommandEntry))

int exec_test_command(char * action, char* args){
    int len_act = strlen((const uint8_t*)action) + 1;// include 0 byte

    for (unsigned int i = 0; i < COMMAND_COUNT; i++) {

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
