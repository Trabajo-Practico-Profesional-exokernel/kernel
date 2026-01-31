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

#define SLEEP_TIME 300000000

#define MAX_INPUT 128
char input_buf[MAX_INPUT];

void interactive_shell_main(void) {
    static char input_buf[MAX_INPUT];
    printf("TEST SHELL interactive: ");
    test_help_man("");

    while (1){
        printf("tester> ");
        // int len = 
        read_line(input_buf, MAX_INPUT);

        char * args= NULL;
        split_by_once((uint8_t*)input_buf, (uint8_t**)&args, ' ');


        if (strncmp((const uint8_t*)input_buf, (const uint8_t*)"q", 2) == 0) {
            printf("Bye!\n");
            break;
        }
        printf("\n");

        exec_test_command(input_buf, args);
    }
}