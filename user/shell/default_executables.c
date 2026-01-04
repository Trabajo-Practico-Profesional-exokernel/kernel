#include "default_executables.h"
#include "std/string.h"
#include "lib.h"
#include "app_names.h"
extern char* _app_names[];
char *DEF_ARGV[] = { "param1","name2", 0 };

#define MAX_ARG 10
#define MAX_ARG_LEN 256

int build_argv(char** argv, char *args, int max_len){
    int argc= 0;
    char * curr_arg = NULL;
    
    // printf("FIRST ARGS ARE '%s'\n", args);
    for(argc = 0; args; argc++) {
        if(argc >= max_len) {
            printf("MORE THAN MAX PARAMS!\n");
            return -1;
        }
        args = extract_once(args, &curr_arg, ' ');
        // printf("AFTR '%s'\n", args);
        size_t arg_len = strlen(curr_arg) + 1; // Same as args- curr_arg in theory...

        if (arg_len > MAX_ARG_LEN){
            printf("ARG LONGER THAN ALLOWED!\n");
            return -1;            
        }

        argv[argc] = curr_arg;
    }
    if (argc == 0){
        argv[0] = 0;
        return 0;
    }
    
    // Finished/reached end..
    argv[argc] = 0;
    return argc -1;
}

int exec_program(char* program_name, char*args){
    int len_name = strlen(program_name) +1;
    for (int ind_program = 0; ind_program < APP_COUNT; ind_program++) {
        if (strncmp(program_name, _app_names[ind_program] , len_name) == 0) {

            // Build argv
            char* argv[MAX_ARG];
            int count = build_argv(&argv[1], args, MAX_ARG);
            if(count < 0){
                printf("Error at parsing parameters.");
                return count;
            }
            argv[0] = program_name;

            count+=1;

            printf("Shell exec %d %s with %d args\n", ind_program,program_name, count);
            for(int argc = 0; argv[argc]; argc++) {
                printf("shell exec arg %d == '%s'\n", argc, argv[argc]);
            }

            int proc_pid= exec(ind_program, &argv[0]);

            printf("Program %s started proc_id is ... %d\n", program_name, proc_pid);
            return proc_pid;
        }
    }
    
    // printf("At exec '%s' program not recognized\n", program_name);
    return ERR_CODE;

}


bool default_executable_check(char* executable_name, char*args, int* out_ret){
    printf("Should check def executable %s\n", executable_name);
    printf("Should check def args %s\n", args);
    printf("Should check def out_ret %s\n", out_ret);
    return false;
}
