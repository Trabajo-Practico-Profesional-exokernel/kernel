#include "lib.h"
#include "string.h"
#include "stdlib.h"
#include "types.h"
#include "stdio.h"
#include "default_executables.h"
#include "command_handler.h"
#include "console/colors.h"
#include "parsers/strutil.h"

#include "environ.h"

// Built-in shell commands (these MUST be handled by the shell, not spawned as programs)
int handle_exit(int argc, char** argv) {
    if(argc > 0){
        exit(atoi(argv[0]));
    }
    exit(0);
    return OK_CODE;
}

int handle_smile(int argc, char** argv) {
    printf("\n");
    printf("         , - ~ ~ ~ - ,           \n");
    printf("     , '               ' ,       \n");
    printf("   ,                       ,     \n");
    printf("  ,    [o]           [o]    ,    \n");
    printf(" ,                           ,   \n");
    printf(" ,             #             ,   \n");
    printf("  ,         \\_ _/           ,   \n");
    printf("   ,                       ,     \n");
    printf("     ,                  , '      \n");
    printf("       ' - , _ _ _ ,  '          \n");
    printf("\n");
    return OK_CODE;
}

int handle_clear(int argc, char** argv) {
    printf("\x1b[2J\x1b[3J\x1b[H");
    return OK_CODE;
}


static char curr_path[256] = {0};

char * get_curr_path(void){
    if(curr_path[0] == 0){
        return NULL;
    }
    // Skip '/'
    return &curr_path[1];
}

int handle_cd(int argc, char** argv) {
    if (argc == 0) {
        printf("Usage: cd <path>\n");
        return ERR_CODE;
    }
    if (chdir(argv[0]) == 0) {
        if (getcwd(curr_path, sizeof(curr_path)) != 0) {
            printf("Error getting pwd\n");
        }

        return OK_CODE;
    }
    printf("cd failed\n");
    return ERR_CODE;
}

int handle_pwd(int argc, char** argv) {
    
    if (getcwd(curr_path, sizeof(curr_path)) == 0) {
        printf("%s\n", curr_path);
        return OK_CODE;
    }
    
    printf("Error getting pwd\n");
    return ERR_CODE;
}


int handle_env(int argc, char** argv) {
    char ** env_list = get_environ_list();
    size_t count = 0;
    while (env_list[count]){
        printf("env at: %d '%s'\n",count, env_list[count]); 
        count++;
    }

    printf("env got %d vars\n",count); 
    return 0;
}


int handle_procls(int argc, char** argv) {
    if (procls() == 0) {
        return OK_CODE;
    }
    return ERR_CODE;
}

int handle_export_env(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: export <name> <value>\n it cannot contain spaces value!");
        return ERR_CODE;
    }

    return setenv(argv[0], argv[1], 1);
}

struct CommandEntry commands[] = {
    // Built-in commands only (must execute in shell process)
    { "exit",      handle_exit },
    { "cd",        handle_cd },
    { "pwd",       handle_pwd },
    { "clear",     handle_clear },
    { "smile",     handle_smile },
    { "procls",    handle_procls },
    { "env",       handle_env },
    { "export",       handle_export_env },
};

// Auto-calculate command count
#define COMMAND_COUNT (sizeof(commands) / sizeof(struct CommandEntry))


int command_default_exec(char* proc_name, char** argv) {
    sys_execv(proc_name, (const char **)argv);
    printf("Error: Cannot execute %s\n", proc_name);
    return 127;    
}


command_handler_t find_command_builtin(char * action){
    // Check for built-in commands first
    int len_act = strlen(action) +1;
    for (unsigned int i = 0; i < COMMAND_COUNT; i++) {
        char* trg_action = commands[i].action_name;
        if (strncmp((const uint8_t*)action, (const uint8_t*)trg_action, len_act) == 0) {
            return commands[i].handler;
        }
    }

    return NULL;    
}
