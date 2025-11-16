#include "lib.h"

#include "app_names.h"
extern char* _app_names[];


#define SLEEP_TIME 300000000

#define MAX_INPUT 128


char input_buf[MAX_INPUT];


void main() {
    static char input_buf[MAX_INPUT];

    printf("SHELL Started registered apps are:\n");
    
    for (int i = 0; i < APP_COUNT; i++) {
        printf("Available runnable %d: %s\n", i, _app_names[i]);
    }

    while (1){
        printf("user> ");
        
        int len = get_string(input_buf, MAX_INPUT);
        
        printf("\nYou typed: %s\n", input_buf);
        sleep(SLEEP_TIME);
        printf("Mock done something..\n");

        if (strncmp(input_buf, "q", 2) == 0) {
            printf("Bye!\n");
            break;
        }
    }
}