#include "default_executables.h"
#include "std/string.h"
#include "lib.h"
#include "app_names.h"
extern char* _app_names[];
char *DEF_ARGV[] = { "param1","name2", 0 };

int exec_program(char* program_name, char*args){
    for (int ind_program = 0; ind_program < APP_COUNT; ind_program++) {
        if (strncmp(program_name, _app_names[ind_program] , strlen(program_name)) == 0) {
            printf("SHOULD RUN AT INDEX! %d: '%s' args '%s'\n", ind_program, program_name, args);
            // int proc_pid= exec(ind_program, 0); // No params test
            int proc_pid= exec(ind_program, &DEF_ARGV[0]);

            printf("Program %s started proc_id is ... %d\n", program_name, proc_pid);
            return proc_pid;
        }
    }
    
    printf("At exec '%s' program not recognized\n", program_name);
    return ERR_CODE;

}


bool default_executable_check(char* executable_name, char*args, int* out_ret){
    printf("Should check def executable %s", executable_name);
    
    return false;
}
