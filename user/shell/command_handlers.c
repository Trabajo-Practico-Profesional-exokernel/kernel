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
    printf("\x1b[2J\x1b[3J\x1b[H");
    return OK_CODE;
}

int handle_mkfs(char* args) {
    printf("Error: MKFS must be run from kernel/boot or dedicated tool.\n");
    return ERR_CODE;
}

int handle_open(char* args) {
    if (args == NULL || strlen(args) == 0) {
        printf("Usage: open <filename>\n");
        return ERR_CODE;
    }
    int fd = open(args, 0); // 0 for default mode
    if (fd < 0) {
        printf("Failed to open file: %s\n", args);
        return ERR_CODE;
    }
    printf("File opened. FD: %d\n", fd);
    return OK_CODE;
}

int handle_read_fs(char* args) {
    char* size_str = strchr(args, ' ');
    if (!size_str) {
        printf("Usage: read <fd> <size>\n");
        return ERR_CODE;
    }
    *size_str = 0;
    size_str++;

    int fd = atoi(args);
    int size = atoi(size_str);
    
    if (size <= 0 || size > 1024) size = 1024; // Limit buffer

    char buffer[1025];
    int bytes = read_fs(fd, buffer, size);
    
    if (bytes < 0) {
        printf("Read error\n");
        return ERR_CODE;
    }
    
    buffer[bytes] = 0;
    printf("Read (%d bytes):\n%s\n", bytes, buffer);
    return OK_CODE;
}

int handle_write_fs(char* args) {
    char* content = strchr(args, ' ');
    if (!content) {
        printf("Usage: write <fd> <string>\n");
        return ERR_CODE;
    }
    *content = 0;
    content++;

    int fd = atoi(args);
    int len = strlen(content);
    
    int bytes = write_fs(fd, content, len);
    
    if (bytes < 0) {
        printf("Write error\n");
        return ERR_CODE;
    }
    
    printf("Written %d bytes to FD %d\n", bytes, fd);
    return OK_CODE;
}

int handle_read(char* args) {
    char* size_str = strchr(args, ' ');
    if (!size_str) {
        printf("Usage: read <fd> <size>\n");
        return ERR_CODE;
    }
    *size_str = 0;
    size_str++;

    int fd = atoi(args);
    int size = atoi(size_str);
    
    if (size <= 0 || size > 1024) size = 1024; // Limit buffer

    char buffer[1025];
    int bytes = read(fd, buffer, size);
    
    if (bytes < 0) {
        printf("Read error\n");
        return ERR_CODE;
    }
    
    buffer[bytes] = 0;
    printf("Read (%d bytes):\n%s\n", bytes, buffer);
    return OK_CODE;
}

int handle_write(char* args) {
    char* content = strchr(args, ' ');
    if (!content) {
        printf("Usage: write <fd> <string>\n");
        return ERR_CODE;
    }
    *content = 0;
    content++;

    int fd = atoi(args);
    int len = strlen(content);
    
    int bytes = write(fd, content, len);
    
    if (bytes < 0) {
        printf("Write error\n");
        return ERR_CODE;
    }
    
    printf("Written %d bytes to FD %d\n", bytes, fd);
    return OK_CODE;
}

int handle_lseek(char* args) {
    char* offset_str = strchr(args, ' ');
    if (!offset_str) {
        printf("Usage: lseek <fd> <offset>\n");
        return ERR_CODE;
    }
    *offset_str = 0;
    offset_str++;

    int fd = atoi(args);
    int offset = atoi(offset_str);

    // Asumiendo que existe un wrapper lseek similar a los otros
    // Si no existe en lib.h, esto fallará al linkear, pero es la lógica correcta.
    int res = lseek(fd, offset, 0); 
    
    if (res < 0) {
        printf("Lseek error\n");
        return ERR_CODE;
    }
    printf("New offset: %d\n", res);
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
    printf("rmdir failed\n");
    return ERR_CODE;
}

int handle_cd(char* args) {
    if (chdir(args) == 0) {
        printf("Changed directory to: %s\n", args);
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
    int fd = atoi(args);
    if (close(fd) == 0) {
        printf("Closed FD %d\n", fd);
        return OK_CODE;
    }
    printf("Close failed\n");
    return ERR_CODE;
}

int handle_link(char* args) {
    char* new_path = strchr(args, ' ');
    if (!new_path) {
        printf("Usage: link <old_path> <new_path>\n");
        return ERR_CODE;
    }
    *new_path = 0;
    new_path++;

    if (link(args, new_path) == 0) {
        printf("Link created: %s -> %s\n", new_path, args);
        return OK_CODE;
    }
    printf("Link failed\n");
    return ERR_CODE;
}

int handle_unlink(char* args) {
    if (sys_rm(args) == 0) {
        printf("Unlinked: %s\n", args);
        return OK_CODE;
    }
    printf("Unlink failed\n");
    return ERR_CODE;
}

int handle_stat(char* args) {
    int result = sys_stat(args);
    if (result == 0) {
        printf("File '%s' exists and is accessible.\n", args);
        return OK_CODE;
    }
    printf("Stat failed or file not found, result: [%d]\n", result);
    return ERR_CODE;
}

int handle_fsck(char* args) {
    printf("FSCK not accessible from shell.\n");
    return ERR_CODE;
}

int handle_pwd(char* args) {
    char path[128];
    
    if (getcwd(path, sizeof(path)) == 0) {
        printGreen(path);
        printf("\n");
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
    if (args == NULL || strlen(args) == 0) {
        printf("Usage: touch <filename>\n");
        return ERR_CODE;
    }

    // Usamos mknod (FS_TYPE_MKNOD) que en el servidor realiza la lógica de crear el archivo
    if (mknod(args, 0, 0) == 0) {
        printf("File created: %s\n", args);
        return OK_CODE;
    }

    printf("Touch failed\n");
    return ERR_CODE;
}

int handle_cat(char* args) {
    if (args == NULL) {
        printf("Usage: cat <filename>\n");
        return ERR_CODE;
    }

    int fd = open(args, 0);
    if (fd < 0) {
        printf("Error opening file\n");
        return ERR_CODE;
    }

    char buf[128];
    int bytes;
    while ((bytes = read(fd, buf, 127)) > 0) {
        buf[bytes] = 0;
        printf("%s", buf);
    }
    printf("\n");

    close(fd);
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
    { "read_fs",   handle_read_fs },
    { "write_fs",  handle_write_fs },
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
    { "touch", handle_touch },
    { "cat",    handle_cat },
    { "pwd",    handle_pwd }
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
