#include "lib.h"
#include "std/string.h"

#include "app_names.h"
extern char* _app_names[];


#define SLEEP_TIME 300000000

#define MAX_INPUT 128


char input_buf[MAX_INPUT];

char * RUN_COMMAND = "exec";

void split_by_once(char* src, char** after_delim, char delimeter){
    char* space_char = strchr(src, delimeter);
    if (space_char == 0){
        *after_delim = ""; //Replace args to an empty string
    } else {
        // Next char is start of args...
        *after_delim = space_char+1;
        *space_char =0; //Replace value by 0 so that input_buf ends here for strncmp!
    }
}


// Just one arg? the progam to exec.. maybe also the args for it .. not for now? 
void handle_exec(char* program_name){
    char * args = NULL;
    split_by_once(program_name, &args, ' ');


    for (int ind_program = 0; ind_program < APP_COUNT; ind_program++) {
        if (strncmp(program_name, _app_names[ind_program] , strlen(program_name)) == 0) {
            printf("SHOULD RUN AT INDEX! %d: '%s' args '%s'\n", ind_program, program_name, args);
            
            int ret= exec(ind_program, &args);

            printf("Return code for %s is ... %d\n", program_name, ret);
            return;
        }
    }
    
    printf("At exec '%s' program not recognized\n", program_name);
}

void main() {
    static char input_buf[MAX_INPUT];

    printf("SHELL Started registered apps are:\n");
    
    for (int i = 0; i < APP_COUNT; i++) {
        printf("Available runnable %d: %s\n", i, _app_names[i]);
    }


    while (1){
        printf("user> ");
        
        int len = get_string(input_buf, MAX_INPUT);

        char * args= NULL; 
        split_by_once(input_buf, &args, ' ');


        if (strncmp(input_buf, "q", 2) == 0) {
            printf("Bye!\n");
            break;
        }
        printf("\n");


        // sleep(SLEEP_TIME);

        if (strncmp(input_buf, RUN_COMMAND, 5) == 0) {
            handle_exec(args);
        } else{
            printf("\nUnknown Command: '%s' args '%s'\n", input_buf, args);
        }

    }
}