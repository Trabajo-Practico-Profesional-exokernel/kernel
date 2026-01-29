#include "coordinator.h"
#include "inc/operations.h"
#include "types.h"
#include "ipc_msgs.h"
#include "constants.h"
#include "syscalls.h"
#include "../../meta/app_names.h"
#include "ipc.h"
#include "server_map.h"
#include "string.h"
#include "stdio.h"
#include "command_handler.h"
#include "types.h"
#include "arch/proc.h"
#include "server_map.h"

Coordinator coordinator;

int32_t get_server_type(int32_t fd, int32_t pid){

    int32_t type = coordinator.fd[pid][fd];

    if (type >= 0 && type < SERVER_COUNT) {
        return type;
    }

    return -1;
}

int32_t set_server_type(int32_t fd, int32_t pid, int32_t type){

    if (type >= 0 && type < SERVER_COUNT) {
        coordinator.fd[pid][fd] = type;
        return SUCCESS;
    }

    return ERROR;
}

int32_t coordinator_noop(void) {
    return -1;
}

int32_t coordinator_putchar(void) {
    return -1;
}

int32_t coordinator_getchar(void) {
    return -1;
}

int32_t coordinator_update(int32_t fd, int32_t pid, int32_t type) {

    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    }

    return set_server_type(fd, pid, type);
}

int32_t coordinator_open(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_close(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    } 

    return server_map_get(&coordinator.server_map, server_type);
}

int32_t coordinator_read(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    } 

    return server_map_get(&coordinator.server_map, server_type);
}

int32_t coordinator_write(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    } 

    return server_map_get(&coordinator.server_map, server_type);
}

int32_t coordinator_lseek(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    } 

    return server_map_get(&coordinator.server_map, server_type);
}

int32_t coordinator_stat(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_dup(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    } 

    return server_map_get(&coordinator.server_map, server_type);
}

int32_t coordinator_pipe(void) {
    return server_map_get(&coordinator.server_map, PIPE);
}

int32_t coordinator_mkdir(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_rmdir(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_chdir(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_cwd(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_ls(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_mknod(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_link(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_unlink(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_chown(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_chmod(void) {
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

static const char * const server_names[] = {
    [COORD]         = "coordinator",
    [FILESYSTEM]    = "filesystem",
    [PIPE]          = "pipe",
    [CONSOLE]       = "console",
    [SHELL]         = "shell"
};

#define GET_SERVER_NAME(type) \
    ((type >= 0 && type < SERVER_COUNT) ? server_names[type] : "unknown")

extern char* _app_names[];

int32_t start_server(Server type) {

    char* program_name = (char*)GET_SERVER_NAME(type);

    int len_name = strlen((const uint8_t*)program_name) + 1;
    for (int ind_program = 0; ind_program < APP_COUNT; ind_program++) {
        
        if (strncmp((const uint8_t*)program_name, (const uint8_t*)_app_names[ind_program], len_name) == 0) {
            char* argv[3];
            argv[0] = program_name;
            argv[1] = 0;
            argv[2] = 0;

            int proc_pid = exec(ind_program, argv);
            if (proc_pid < 0) {
                printf("[Coordinator] Failed to start %s. Error: %d\n", program_name, proc_pid);
                return ERROR;
            }

            server_map_set(&coordinator.server_map, type, proc_pid);

            return SUCCESS;
        }
    }
    return ERROR;
}


void init_fds() {
    for (int i = 0; i < PROCS_MAX; i++) {
        for (int j = 0; j < MAX_FILES; j++) {
            coordinator.fd[i][j] = -1;
        }
    }
}

void init_coordinator(){
    server_map_init(&coordinator.server_map);
    server_map_set(&coordinator.server_map, COORD, getpid());
    init_fds();
}

void init_servers(){
    start_server(FILESYSTEM);
    start_server(SHELL);
}
