#include "parsers/strutil.h"
#include "console/debug.h"
#include "string.h"
#include "stdlib.h"
#include "test_shell/command_handler.h"
#include "interactive_test_commands.h"
#include "test_shell/interactive_info_logging.h"


int test_handle_run_shell(char* args) {
    start_shell();
    return 0;
}

int test_handle_run_tests(char* args) {
    run_tests();
    return 0;
}

int test_start_cpus(char* args) {
    printf("Starting secondary cpus\n");
    start_cpus();
    wait_cpus_started();
    printf("...All cpus started\n");

    return 0;
}


int handle_enable_log_irq(char* args) {
    return enable_clock_yield_logging(args);    
}

int handle_disable_log_irq(char* args) {
    return disable_clock_yield_logging();    
}

int handle_enable_interactive_irq(char* args) {
    return enable_clock_yield_interactive(args);    
}

int handle_disable_interactive_irq(char* args) {
    return disable_clock_yield_interactive();    
}


int handle_sched_yield(char *args){
    do_sched_yield(args);
    return 0;
}



int test_help_man(char* args);

struct CommandEntry commands[] = {
    { "run_shell", test_handle_run_shell,
    "corre la shell de usuario y podes probar programas de la misma"
    },
    { "run_tests", test_handle_run_tests,
    "recibe: <args para los tests>, corre uno o mas tests predefinidos del kernel, en testing.c"
    },
    { "start_cpus", test_start_cpus,
    "empieza el sistema multicore, levanta/desbloquea los cores"
    },
    { "irq_log_on", handle_enable_log_irq,
    "habilita logs de informacion en cada interrupcion por clock"
    },
    { "irq_log_off", handle_disable_log_irq,
    "deshabilita logs de informacion en cada interrupcion por clock"},
    { "irq_to_shell_on", handle_enable_interactive_irq,
    "habilita que en cada interrupcion por clock se vuelva a la test shell"
    },
    { "irq_to_shell_off", handle_disable_interactive_irq,
    "deshabilita que en cada interrupcion por clock se vuelva a la test shell"
    },
    { "sched_yield", handle_sched_yield,
    "Corre sched_yield... volviendo al scheduler basicamente"
    },
    { "add_proc", handle_create_proc,
    "Con parametros '<program_name> <args>' Agrega al scheduler un proceso para ser ejecutado"
    },
    { "help", test_help_man, "Da informacion general de comandos"}
    
};
// Auto-calculate command count
#define COMMAND_COUNT (sizeof(commands) / sizeof(struct CommandEntry))


int test_help_man(char* args) {
    printf("Podes correr:\n");
    for (unsigned int i = 0; i < COMMAND_COUNT; i++) {
        printf("'%s' %s\n",commands[i].action_name, commands[i].description);
    }
    return 0;
}


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
