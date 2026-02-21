#include "interactive_test_commands.h"
#include "testing.h"
#include "test_shell/command_handler.h"



int test_handle_run_tests(char* args) {
    main_tests();
    return 0;
}

void init_test_commands(void){
    add_test_command((struct CommandEntry){
        .action_name = "run_tests",
        .handler = test_handle_run_tests,
        .description = "recibe: <args para los tests>, corre uno o mas tests predefinidos del kernel, en testing.c"
    });
}


