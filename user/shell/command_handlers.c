#include "lib.h"
#include "string.h"
#include "stdlib.h"
#include "types.h"
#include "stdio.h"
#include "default_executables.h"
#include "command_handler.h"
#include "console/colors.h"
#include "parsers/strutil.h"

// Just one arg?
// the progam to exec.. maybe also the args for it .. not for now?
int start_program(char* program_name){
    char * args = NULL;
    split_by_once((uint8_t*)program_name, (uint8_t**)&args, ' ');
    // printf("START  %s with args %s\n", program_name, args);

    return exec_program(program_name, args);
}

int handle_exec(char* args){
    int pid_child = start_program(args);
    if (pid_child == -1){ 
        return ERR_CODE;
    }
    // Wait for the child!
    int ret_code= wait(pid_child);
    printf("Waited for proc %d! exited with code %d\n", pid_child, ret_code);

    return ret_code;
}

int handle_wait(char* args){

    long pid_waited = strtol((const uint8_t*)args, NULL, 0);
    int ret_code= wait(pid_waited);
    printf("Waited for proc %d! exited with code %d\n", pid_waited, ret_code);
    return ret_code;
}

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

int handle_mkfs(char* args) {
    (void)args;
    printf("Error: MKFS must be run from kernel/boot or dedicated tool.\n");
    return ERR_CODE;
}

int handle_open(char* args) {
    if (args == NULL || strlen((const uint8_t*)args) == 0) {
        printf("Usage: open <filename>\n");
        return ERR_CODE;
    }

    int fd = open(args, 0); 
    
    if (fd < 0) {
        printf("Failed to open file: %s\n", args);
        return ERR_CODE;
    }
    printf("File opened. FD: %d\n", fd);
    return OK_CODE;
}

int handle_read(char* args) {
    char* size_str = split_arg(args);
    
    if (!args || !size_str) {
        printf("Usage: read <fd> <size>\n");
        return ERR_CODE;
    }

    int fd = atoi((const uint8_t*)args);
    int size = atoi((const uint8_t*)size_str);
    
    if (size <= 0 || size > 1024) size = 1024; // Buffer limit

    char buffer[1025];
    int bytes = read(fd, buffer, size);
    
    if (bytes < 0) {
        printf("Read error or EOF\n");
        return ERR_CODE;
    }
    
    buffer[bytes] = 0; // Null terminate para imprimir
    printf("Read (%d bytes):\n%s\n", bytes, buffer);
    return OK_CODE;
}

int handle_write(char* args) {
    char* content = split_arg(args);

    if (!args || !content) {
        printf("Usage: write <fd> <string>\n");
        return ERR_CODE;
    }

    int fd = atoi((const uint8_t*)args);
    int len = strlen((const uint8_t*)content);
    
    int bytes = write(fd, content, len);
    
    if (bytes < 0) {
        printf("Write error\n");
        return ERR_CODE;
    }
    
    printf("Written %d bytes to FD %d\n", bytes, fd);
    return OK_CODE;
}

int handle_lseek(char* args) {
    char* offset_str = split_arg(args);

    if (!args || !offset_str) {
        printf("Usage: lseek <fd> <offset>\n");
        return ERR_CODE;
    }

    int fd = atoi((const uint8_t*)args);
    int offset = atoi((const uint8_t*)offset_str);
    
    int res = lseek(fd, offset, 0); 
    
    if (res < 0) {
        printf("Lseek error\n");
        return ERR_CODE;
    }
    printf("New offset: %d\n", res);
    return OK_CODE;
}

int handle_mkdir(char* args) {
    if (!args || strlen((const uint8_t*)args) == 0) {
        printf("Usage: mkdir <path>\n");
        return ERR_CODE;
    }
    if (mkdir(args) == 0) {
        printf("mkdir success: %s\n", args);
        return OK_CODE;
    }
    printf("mkdir failed\n");
    return ERR_CODE;
}

int handle_rmdir(char* args) {
    if (!args || strlen((const uint8_t*)args) == 0) {
        printf("Usage: rmdir <path>\n");
        return ERR_CODE;
    }
    if (rmdir(args) == 0) {
        printf("rmdir success: %s\n", args);
        return OK_CODE;
    }
    printf("rmdir failed\n");
    return ERR_CODE;
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

int handle_close(char* args) {
    if (args == NULL) {
        printf("Usage: close <fd>\n");
        return ERR_CODE;
    }
    int fd = atoi((const uint8_t*)args);
    if (close(fd) == 0) {
        printf("Closed FD %d\n", fd);
        return OK_CODE;
    }
    printf("Close failed\n");
    return ERR_CODE;
}

int handle_link(char* args) {
    char* new_path = split_arg(args);

    if (!args || !new_path) {
        printf("Usage: link <old_path> <new_path>\n");
        return ERR_CODE;
    }

    if (link(args, new_path) == 0) {
        printf("Link created: %s -> %s\n", new_path, args);
        return OK_CODE;
    }
    printf("Link failed\n");
    return ERR_CODE;
}

int handle_unlink(char* args) {
    if (!args || strlen((const uint8_t*)args) == 0) {
        printf("Usage: unlink <path>\n");
        return ERR_CODE;
    }
    if (unlink(args) == 0) {
        printf("Unlinked: %s\n", args);
        return OK_CODE;
    }
    printf("Unlink failed\n");
    return ERR_CODE;
}

int handle_stat(char* args) {
    if (!args || strlen((const uint8_t*)args) == 0) {
        printf("Usage: stat <path>\n");
        return ERR_CODE;
    }
    
    int result = stat(args);
    if (result == 0) {
        return OK_CODE;
    }
    printf("Stat failed, code: %d\n", result);
    return ERR_CODE;
}

int handle_fsck(char* args) {
    (void)args;
    printf("FSCK not accessible from shell.\n");
    return ERR_CODE;
}

int handle_pwd(char* args) {
    (void)args;
    char path[256];
    
    if (getcwd(path, sizeof(path)) == 0) {
        printf("%s\n", path); // O printGreen si lo tienes definido
        return OK_CODE;
    }
    
    printf("Error getting pwd\n");
    return ERR_CODE;
}

int handle_ls(char* args) {
    if (ls(args) == 0) {
        return OK_CODE;
    }
    return ERR_CODE;
}

int handle_touch(char* args) {
    if (args == NULL || strlen((const uint8_t*)args) == 0) {
        printf("Usage: touch <filename>\n");
        return ERR_CODE;
    }

    if (mknod(args, 0, 0) == 0) {
        printf("File created: %s\n", args);
        return OK_CODE;
    }

    printf("Touch failed\n");
    return ERR_CODE;
}

int handle_cat(char* args) {
    if (args == NULL || strlen((const uint8_t*)args) == 0) {
        printf("Usage: cat <filename>\n");
        return ERR_CODE;
    }

    int fd = open(args, 0); // 0 = O_RDONLY (generalmente)
    if (fd < 0) {
        printf("Error opening file: %s\n", args);
        return ERR_CODE;
    }

    char buf[128];
    int bytes;
    // Bucle de lectura
    while ((bytes = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = 0;
        printf("%s", buf);
    }
    printf("\n");

    close(fd);
    return OK_CODE;
}

int handle_chown(char* args) {
    char* uid_str = split_arg(args);
    if (!args || !uid_str) {
        printf("Usage: chown <path> <uid> <gid>\n");
        return ERR_CODE;
    }

    char* gid_str = split_arg(uid_str);
    if (!gid_str) {
        printf("Usage: chown <path> <uid> <gid>\n");
        return ERR_CODE;
    }

    char* path = args;
    uint32_t uid = atoi((const uint8_t*)uid_str);
    uint32_t gid = atoi((const uint8_t*)gid_str);

    if (chown(path, uid, gid) == 0) {
        printf("chown success: %s -> %d:%d\n", path, uid, gid);
        return OK_CODE;
    }
    printf("Error with chown\n");
    return ERR_CODE;
}

int handle_chmod(char* args) {
    char* mode_str = split_arg(args);
    
    if (!args || !mode_str) {
        printf("Usage: chmod <path> <mode>\n");
        return ERR_CODE;
    }

    char* path = args;
    uint32_t mode = atoi((const uint8_t*)mode_str);

    if (chmod(path, mode) == 0) {
        printf("chmod success: %s -> %d\n", path, mode);
        return OK_CODE;
    }
    printf("Error with chmod\n");
    return ERR_CODE;
}

struct CommandEntry commands[] = {
    // Existing
    { "exec",      handle_exec },
    { "start",     start_program },
    { "wait",      handle_wait },
    { "exit",      handle_exit },
    { "smile",     handle_smile },
    { "clear",     handle_clear },
    { "mkfs",      handle_mkfs },
    { "open",      handle_open },
    { "read",      handle_read },
    { "write",     handle_write },
    { "lseek",     handle_lseek },
    { "mkdir",     handle_mkdir },
    { "rmdir",     handle_rmdir },
    { "cd",        handle_cd },
    { "close",     handle_close },
    { "link",      handle_link },
    { "unlink",    handle_unlink },
    { "stat",      handle_stat },
    { "fsck",      handle_fsck },
    { "ls",        handle_ls },
    { "touch",     handle_touch },
    { "cat",       handle_cat },
    { "pwd",       handle_pwd },
    { "chown",     handle_chown },
    { "chmod",     handle_chmod },
};
// Auto-calculate command count
#define COMMAND_COUNT (sizeof(commands) / sizeof(struct CommandEntry))

int exec_command(char * action, char* args){
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
    
    int ret_code = ERR_CODE;
    if (default_executable_check(action, args, &ret_code)){
        return ret_code;
    }
    
    printf("\nUnknown Command: '%s' args '%s'\n", action, args);
    return ret_code;
}
