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
#include "direct_printf.h"


Coordinator coordinator;

int32_t is_server_alive(int32_t type_server){
    int32_t server_pid = server_map_get(&coordinator.server_map, type_server);
    return (alive(server_pid));
}


int32_t handle_dead_server(int32_t type_server){

    for (int i = 0; i < PROCS_MAX; i++) {
        for (int j = 0; j < MAX_FILES; j++) {

            if (coordinator.fd[i][j].type_server == type_server){
                coordinator.fd[i][j].fd = -1;
                coordinator.fd[i][j].type_server = -1;
            }
        
        }
    }

    start_server(type_server);
}

int32_t get_server_type(int32_t fd, int32_t pid) {
    // printf("[COORD] get_server_type query -> AppPID: %d | AppFD: %d\n", pid, fd);

    if (pid < 0 || pid >= PROCS_MAX || fd < 0 || fd >= MAX_FILES) {
        //direct_printf("[COORD] ERROR: get_server_type out of bounds (PID: %d, FD: %d)\n", pid, fd);
        return -1;
    }
    
    int32_t type = coordinator.fd[pid][fd].type_server;
    
    // Solo imprimir si encontramos algo util, para no spamear con -1
    if (type >= 0 && type < SERVER_COUNT) {
        // printf("[COORD] get_server_type MATCH -> AppPID: %d | FD: %d is managed by ServerType: %d\n", pid, fd, type);
        return type;
    }
    
    // printf("[COORD] get_server_type EMPTY -> AppPID: %d | FD: %d is unmapped (-1)\n", pid, fd);
    return -1;
}

int32_t get_server_fd(int32_t pid_app, int32_t fd, int32_t server_pid) {
    if (pid_app < 0 || pid_app >= PROCS_MAX || fd < 0 || fd >= MAX_FILES) {
        //direct_printf("[COORD] ERROR: get_server_fd out of bounds (PID: %d, FD: %d)\n", pid_app, fd);
        return -1;
    }

    int32_t server_fd = coordinator.fd[pid_app][fd].fd;
    
    // printf("[COORD] Translation: AppPID: %d uses AppFD: %d -> Translates to ServerFD: %d\n", pid_app, fd, server_fd);
    
    return server_fd;
}


int32_t get_server_real_fd(int32_t pid, int32_t fd, int32_t type){
    if (pid < 0 || pid >= PROCS_MAX || fd < 0 || fd >= MAX_FILES) {
        //direct_printf("[COORD] ERROR: get_server_fd out of bounds (PID: %d, FD: %d)\n", pid, fd);
        return -1;
    }

    for (int32_t real_fd = 0; real_fd < MAX_FILES; real_fd++) {
        // Buscamos coincidencia exacta para borrar
        if (coordinator.fd[pid][real_fd].fd == fd && 
            coordinator.fd[pid][real_fd].type_server == type) 
        {
            return real_fd;
        }
    }
    return ERROR;
}

int32_t set_server_type(int32_t server_fd, int32_t pid_app, int32_t pid_server, int32_t state, int trg_fd) {
    int32_t server_type = -1;
    for (int32_t i = 0; i < SERVER_COUNT; i++) {
        if (coordinator.server_map.pids[i] == pid_server) {
            server_type = i;
            break;
        }
    }
    //direct_printf("[COORDINATOR SET SERVER ] server type %d trg fd: %d,state : %d\n", server_type, trg_fd, state);

    if (server_type < 0) return ERROR;

    if (state < 0) {
        for (int32_t i = 0; i < MAX_FILES; i++) {
            if (coordinator.fd[pid_app][i].fd == server_fd && coordinator.fd[pid_app][i].type_server == server_type) {
                //direct_printf("[COORD] Cierre FD: PID [%d] liberando FD Virtual [%d]\n", pid_app, i);
                coordinator.fd[pid_app][i].fd = -1;
                coordinator.fd[pid_app][i].type_server = -1;
                return SUCCESS;
            }
        }
        return ERROR;
    } else {

        if (trg_fd<0){ // No target fd.. look for first available
            for (int32_t i = 0; i < MAX_FILES; i++) {
                if (coordinator.fd[pid_app][i].type_server == -1) {
                    //direct_printf("[COORD] Apertura FD: PID [%d] mapeando FD Real [%d] a FD Virtual [%d] (Server %d)\n", 
                    //    pid_app, server_fd, i, server_type);
                    coordinator.fd[pid_app][i].fd = server_fd;
                    coordinator.fd[pid_app][i].type_server = server_type;
                    return i;
                }
            }
        } else {
            //direct_printf("[COORD] Apertura FD: PID [%d] mapeando FD Real [%d] a FD Virtual [%d] (Server %d)\n", 
            //    pid_app, server_fd, trg_fd, server_type);
            // IF it is kernel server i.e console one then no problem.. but > 1 is other server
            // Close is needed.
            if(coordinator.fd[pid_app][trg_fd].type_server > 1){ 
                //direct_printf("FD Had a server? %d \n", coordinator.fd[pid_app][trg_fd].type_server);
                return ERROR;
            }

            coordinator.fd[pid_app][trg_fd].fd = server_fd;
            coordinator.fd[pid_app][trg_fd].type_server = server_type;
            return trg_fd;
        }

        
        return ERROR;
    }
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

int32_t coordinator_update_fd(int32_t fd, int32_t pid, int32_t type, int32_t state, int trg_fd) {
    //direct_printf("[COORDINATOR UPDATE FD ] fd: %d for pid: %d type? %d state: %d trg fd? %d\n",
    // fd,pid, type, state, trg_fd);
    if (fd < 0 || fd >= MAX_FILES || pid < 0 || pid >= PROCS_MAX) {
        return ERROR;
    }

    reset_current_client_pid();
    return set_server_type(fd, pid, type, state, trg_fd);
}

int32_t coordinator_open(void) {

    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }

    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_close(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    } 

    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }

    int32_t ret = server_map_get(&coordinator.server_map, server_type);
    if(ret == 0 ){
        coordinator.fd[pid][fd].type_server = -1;
        return -1;
    }
    return ret;
}

int32_t coordinator_read(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    } 

    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }

    return server_map_get(&coordinator.server_map, server_type);
}

int32_t coordinator_write(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    } 

    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }

    return server_map_get(&coordinator.server_map, server_type);
}

int32_t coordinator_lseek(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    }

    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }

    return server_map_get(&coordinator.server_map, server_type);
}

int32_t coordinator_stat(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_dup(int32_t fd, int32_t pid) {
    int32_t server_type = get_server_type(fd, pid);
    if (server_type < 0){
        return -1;
    } 

    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }

    return server_map_get(&coordinator.server_map, server_type);
}

int32_t coordinator_pipe(void) {
    int server_type = PIPE;
    if (!is_server_alive(server_type)){

        handle_dead_server(server_type);
        return ERROR;
    }

    return server_map_get(&coordinator.server_map, PIPE);
}

int32_t coordinator_mkdir(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_rmdir(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_chdir(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_cwd(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_ls(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_mknod(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_link(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_unlink(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_chown(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_chmod(void) {
    int server_type = FILESYSTEM;
    if (!is_server_alive(server_type)){
        handle_dead_server(server_type);
        return ERROR;
    }
    return server_map_get(&coordinator.server_map, FILESYSTEM);
}

int32_t coordinator_fork(int32_t child_pid, int32_t father_pid) {
    //direct_printf("[COORD] coordinator_fork: Clonando tabla de Padre [%d] a Hijo [%d]\n", father_pid, child_pid);
    if (child_pid < 0 || child_pid >= PROCS_MAX || father_pid < 0 || father_pid >= PROCS_MAX) {
        return ERROR;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        coordinator.fd[child_pid][i].fd = coordinator.fd[father_pid][i].fd;
        coordinator.fd[child_pid][i].type_server = coordinator.fd[father_pid][i].type_server;
        if (coordinator.fd[child_pid][i].type_server != -1) {
            //direct_printf("[COORD] Heredado: Hijo [%d] FD Virtual [%d] -> FD Real [%d] Server [%d]\n", 
            //       child_pid, i, coordinator.fd[child_pid][i].fd, coordinator.fd[child_pid][i].type_server);
        }
    }
    return SUCCESS;
}



int32_t coordinator_close_all(int32_t server_type){
    if (server_type == FILESYSTEM || server_type == PIPE) {
        if (!is_server_alive(server_type)){
            handle_dead_server(server_type);
            return ERROR;
        }
        return server_map_get(&coordinator.server_map, server_type);
    }

    return ERROR;

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

            int proc_pid = sys_exec(argv); // No necesitamos pasar el índice del programa, el exec se encargará de buscarlo por nombre
            if (proc_pid < 0) {
                //direct_printf("[Coordinator] Failed to start %s. Error: %d\n", program_name, proc_pid);
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
        coordinator.fd[i][0].fd = 0;
        coordinator.fd[i][0].type_server = KERNEL;
        coordinator.fd[i][1].fd = 1;
        coordinator.fd[i][1].type_server = KERNEL;
        for (int j = 2; j < MAX_FILES; j++) {
            coordinator.fd[i][j].fd = -1;
            coordinator.fd[i][j].type_server = -1;
        }
    }
}

void init_coordinator(){
    server_map_init(&coordinator.server_map);
    server_map_set(&coordinator.server_map, COORD, getpid());
    init_fds();
}

void init_servers(){
    start_server(PIPE);
    start_server(FILESYSTEM);
    start_server(SHELL);
}
