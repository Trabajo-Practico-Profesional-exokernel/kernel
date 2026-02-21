#include "lib.h"
#include "string.h"
#include "stdlib.h"
#include "utils.h"
#include "app_names.h"
#include "console/debug.h"
#include "parsers/strutil.h"
#include "shell_parser.h"
#include "shell_executor.h"

extern char* _app_names[];
// Own includes
#include "command_handler.h"

#define SLEEP_TIME 300000000

#define MAX_INPUT 256
char input_buf[MAX_INPUT];

void main() {
    static char input_buf[MAX_INPUT];
    disable_debug_print();
    printf("SHELL Started registered apps are:\n");

    for (int i = 0; i < APP_COUNT; i++) {
        printf("Available runnable %d: %s\n", i, _app_names[i]);
    }

    printf("\nSupported features:\n");
    printf("  - Pipes: cmd1 | cmd2 | cmd3\n");
    printf("  - Input redirection: cmd < file\n");
    printf("  - Output redirection: cmd > file or cmd >> file\n");
    printf("  - Error redirection: cmd 2> file or cmd 2>> file\n");
    printf("\n");

    while (1){
        printf("user> ");
        read_line(input_buf, MAX_INPUT);

        if (strncmp((const uint8_t*)input_buf, (const uint8_t*)"q", 2) == 0) {
            printf("Bye!\n");
            break;
        }
        
        if (strlen((const uint8_t*)input_buf) == 0) {
            continue; // Skip empty lines
        }
        
        printf("\n");

        // Parse the command line
        parse_result_t parse_result = parse_command_line(input_buf);
        
        if (parse_result.parse_error) {
            printf("Parse error: %s\n", parse_result.error_msg);
        } else {
            // Execute the parsed pipeline
            exec_result_t exec_result = execute_pipeline(&parse_result.pipeline);
            
            if (exec_result.execution_error) {
                printf("Execution error: %s\n", exec_result.error_msg);
            } else if (exec_result.exit_code != 0) {
                // Optionally print exit code if non-zero
                // printf("[Exit code: %d]\n", exec_result.exit_code);
            }
            
            // Free parsed resources
            free_parse_result(&parse_result);
        }
    }
}