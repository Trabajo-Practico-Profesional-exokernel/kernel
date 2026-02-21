#include "lib.h"
#include "string.h"
#include "stdlib.h"
#include "types.h"
#include "stdio.h"
#include "default_executables.h"
#include "command_handler.h"
#include "console/colors.h"
#include "parsers/strutil.h"

// Built-in shell commands (these MUST be handled by the shell, not spawned as programs)

int handle_exit(char* args) {
    (void)args;
    exit(0);
    return OK_CODE;
}

int handle_smile(char* args) {
    (void)args;
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

int handle_clear(char* args) {
    (void)args;
    printf("\x1b[2J\x1b[3J\x1b[H");
    return OK_CODE;
}

int handle_cd(char* args) {
    if (!args || strlen((const uint8_t*)args) == 0) {
        printf("Usage: cd <path>\n");
        return ERR_CODE;
    }
    if (chdir(args) == 0) {
        return OK_CODE;
    }
    printf("cd failed\n");
    return ERR_CODE;
}

int handle_pwd(char* args) {
    (void)args;
    char path[256] = {0};
    
    if (getcwd(path, sizeof(path)) == 0) {
        printf("%s\n", path);
        return OK_CODE;
    }
    
    printf("Error getting pwd\n");
    return ERR_CODE;
}

int handle_procls(char* args) {
    (void)args;
    if (procls() == 0) {
        return OK_CODE;
    }
    return ERR_CODE;
}

struct CommandEntry commands[] = {
    // Built-in commands only (must execute in shell process)
    { "exit",      handle_exit },
    { "cd",        handle_cd },
    { "pwd",       handle_pwd },
    { "clear",     handle_clear },
    { "smile",     handle_smile },
    { "procls",    handle_procls },
};

// Auto-calculate command count
#define COMMAND_COUNT (sizeof(commands) / sizeof(struct CommandEntry))

int exec_command(char * action, char* args){
    int len_act = strlen((const uint8_t*)action) + 1;

    // Check for built-in commands first
    for (unsigned int i = 0; i < COMMAND_COUNT; i++) {
        char* trg_action = commands[i].action_name;
        if (strncmp((const uint8_t*)action, (const uint8_t*)trg_action, len_act) == 0) {
            if(!args){
                args = "";
            }
            return commands[i].handler(args);
        }
    }
    
    // Not a built-in, try to execute as external program
    int ret_code = ERR_CODE;
    if (default_executable_check(action, args, &ret_code)){
        return ret_code;
    }
    
    printf("\nUnknown Command: '%s' args '%s'\n", action, args);
    return ret_code;
}
