#include "lib.h"
#include "string.h"
#include "stdlib.h"
#include "utils.h"
#include "app_names.h"
#include "console/debug.h"
#include "parsers/strutil.h"

extern char* _app_names[];

// Own includes
#include "command_handler.h"

#define SLEEP_TIME 300000000

#define MAX_INPUT 128
char input_buf[MAX_INPUT];

void main() {
    static char input_buf[MAX_INPUT];
    disable_debug_print();
    printf("SHELL Started registered apps are:\n");

    for (int i = 0; i < APP_COUNT; i++) {
        printf("Available runnable %d: %s\n", i, _app_names[i]);
    }


    while (1){
        printf("user> ");
        
        // int len = 
        read_line(input_buf, MAX_INPUT);

        char * args= NULL; 
        split_by_once(input_buf, &args, ' ');


        if (strncmp(input_buf, "q", 2) == 0) {
            printf("Bye!\n");
            break;
        }
        printf("\n");

        exec_command(input_buf, args);
        // sleep(SLEEP_TIME);
    }
}
