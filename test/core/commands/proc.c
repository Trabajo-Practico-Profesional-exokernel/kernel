#include "sched.h"
#include "proc.h"
// #include "fd.h"
#include "arch/logging.h"
#include "arch/mem_layout.h"
#include "arch_inc/mem_constants.h" //defines perms like PAGE_R and so on.
// #include "meta/apps_info.h" // Include auto generated app_info and indexs for apps  
#include "constants.h"
#include "arch/cpus.h"
#include "stdio.h"
#include "console/debug.h"
#include "console_files.h"
#include "arch_inc/trap_constants.h"

#include "interactive_test_commands.h"

#include "string.h"
#include "stdlib.h"
#include "parsers/strutil.h"

// extern struct AppBinaryInfo _binary_apps[];
#include "proc_disk_loading.h"
#include "arch/arch_init.h"
#include "test_shell/command_handler.h"



/// For now defined on sched_init.c
extern struct Proc * create_process(char* proc_name, char ** argv);
extern struct Proc * create_process_from_ind(int ind, char ** argv);

char *DEF_ARGV_TEST[] = { "sh_prog","parameter1", 0 };
extern char *APP_NAME_COORDINATOR; //= "coordinator";


int do_sched_yield(char * args){
    enable_timer_interrupts();
    sched_yield();
    return 0;
}

int load_processes_headers(char *args){
    init_proc_headers();
    return 0;
}


int start_shell(char *args){

    struct Proc * first_main_proc = create_process(APP_NAME_COORDINATOR, DEF_ARGV_TEST);
    coordinator_PID = first_main_proc->pid;
    enable_timer_interrupts();
    sched_yield();
    return 0;
}


#define MAX_ARG 10
#define MAX_ARG_LEN 128

int build_interactive_argv(char** argv, char *args, int max_len){
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


int handle_create_proc(char*program_name){
    char * args = NULL;
    split_by_once((uint8_t*)program_name, (uint8_t**)&args, ' ');


    int ind_program = get_app_from_name(program_name);

    if(ind_program < 0){
        printf("No such program '%s' to run!\n", program_name);
        return -1;
    }
    
    int len_name = strlen(program_name) +1;
    // Build argv
    char* argv[MAX_ARG];
    int count = build_interactive_argv(&argv[1], args, MAX_ARG);
    if(count < 0){
        printf("Error at parsing parameters.\n");
        return count;
    }
    argv[0] = program_name;

    count+=1;
    printf("DO Create proc exec %d %s with %d args\n", ind_program,program_name, count);
    struct Proc * new_proc = create_process_from_ind(ind_program, &argv);
    
    return new_proc->pid;
}





void init_proc_commands(void){
    add_test_command((struct CommandEntry){
        .action_name = "run_shell",
        .handler = start_shell,
        .description = "corre la shell de usuario y podes probar programas de la misma"
    });

    add_test_command((struct CommandEntry){
        .action_name = "sched_yield",
        .handler = do_sched_yield,
        .description = "Corre sched_yield... volviendo al scheduler basicamente"
    });
    add_test_command((struct CommandEntry){
        .action_name = "load_procs",
        .handler = load_processes_headers,
        .description = "Carga los headers de los procesos para que puedan ser corridos por el scheduler"
    });

    add_test_command((struct CommandEntry){
        .action_name = "add_proc",
        .handler = handle_create_proc,
        .description = "Con parametros '<program_name> <args>' Agrega al scheduler un proceso para ser ejecutado"
    });
}

