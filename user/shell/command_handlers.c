#include "lib.h"
#include "std/string.h"
#include "inc/types.h"
#include "std/printf.h"
#include "default_executables.h"
#include "command_handler.h"


// Just one arg? the progam to exec.. maybe also the args for it .. not for now? 
int start_program(char* program_name){
    char * args = NULL;
    split_by_once(program_name, &args, ' ');

    // printf("START  %s with args %s\n", program_name, args);

    return exec_program(program_name, args);
}

int handle_exec(char* args){
    int pid_child = start_program(args);

    if (pid_child == -1){ // lets assume a proc cannot hav -1 as pid? maybe for now this works lol
        return ERR_CODE;
    }
    // Wait for the child!
    int ret_code= wait(pid_child);
    printf("Waited for proc %d! exited with code %d\n", pid_child, ret_code);

    return ret_code;
}

int handle_wait(char* args){

    long pid_waited = strtol(args, NULL, 0);
    int ret_code= 0;
    // int ret_code= wait(pid_waited);
    printf("Waited for proc %d! exited with code %d\n", pid_waited, ret_code);
}

int handle_exit(char* args) {
    exit(0);
    return OK_CODE;
}

int handle_smile(char* args) {
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
    printf("\x1b[2J\x1b[H");
    return OK_CODE;
}

int handle_mkfs(char* args) {
    // TODO: Implement mkfs
    return OK_CODE;
}

int handle_open(char* args) {
    // TODO: Implement open file
    return OK_CODE;
}

int handle_read(char* args) {
    // TODO: Implement read file
    return OK_CODE;
}

int handle_write(char* args) {
    // TODO: Implement write file
    return OK_CODE;
}

int handle_lseek(char* args) {
    // TODO: Implement lseek
    return OK_CODE;
}

int handle_mkdir(char* args) {
    if (mkdir(args) == 0) {
        printf("mkdir success: %s\n", args);
        return OK_CODE;
    }
    printf("mkdir failed\n");
    return ERR_CODE;
}

int handle_rmdir(char* args) {
    if (rmdir(args) == 0) {
        printf("rmdir success: %s\n", args);
        return OK_CODE;
    }
    printf("mkdir failed\n");
    return ERR_CODE;
}

int handle_cd(char* args) {
    // TODO: Implement change directory
    return OK_CODE;
}

int handle_close(char* args) {
    // TODO: Implement close file
    return OK_CODE;
}

int handle_link(char* args) {
    // TODO: Implement link
    return OK_CODE;
}

int handle_unlink(char* args) {
    // TODO: Implement unlink
    return OK_CODE;
}

int handle_stat(char* args) {
    // TODO: Implement stat
    return OK_CODE;
}

int handle_fsck(char* args) {
    // TODO: Implement fsck
    return OK_CODE;
}

int handle_ls(char* args) {
    // TODO: Implement ls
    return OK_CODE;
}

int handle_create(char* args) {
    // TODO: Implement create file
    return OK_CODE;
}

int handle_cat(char* args) {
    // TODO: Implement cat file
    return OK_CODE;
}


struct CommandEntry commands[] = {
    // Existing
    { "exec",   handle_exec },
    { "start",  start_program },
    //{ "msg",    send },
    { "wait",   handle_wait },
    
    // Filesystem & Shell Utilities
    { "exit",   handle_exit },
    { "smile",  handle_smile },
    { "clear",  handle_clear },
    { "mkfs",   handle_mkfs },
    { "open",   handle_open },
    { "read",   handle_read },
    { "write",  handle_write },
    { "lseek",  handle_lseek },
    { "mkdir",  handle_mkdir },
    { "rmdir",  handle_rmdir },
    { "cd",     handle_cd },
    { "close",  handle_close },
    { "link",   handle_link },
    { "unlink", handle_unlink },
    { "stat",   handle_stat },
    { "fsck",   handle_fsck },
    { "ls",     handle_ls },
    { "create", handle_create },
    { "cat",    handle_cat }
};

// Auto-calculate command count
#define COMMAND_COUNT (sizeof(commands) / sizeof(struct CommandEntry))

int exec_command(char * action, char* args){
    int len_act = strlen(action) + 1;// include 0 byte

    for (int i = 0; i < COMMAND_COUNT; i++) {

        char* trg_action = commands[i].action_name;
        
        if (strncmp(action, trg_action, len_act) == 0) {
            if(!args){ // Fill with empty if not defined.
                args = "";
            }
            return commands[i].handler(args);
        }
    }
    
    int ret_code = ERR_CODE;

    if (default_executable_check(action, args, &ret_code)){
        return ret_code;
    }
    
    printf("\nUnknown Command: '%s' args '%s'\n", action, args);
    return ret_code;
}
