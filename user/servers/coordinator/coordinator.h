#ifndef COORDINATOR
#define COORDINATOR

#include "inc/operations.h"
#include "types.h"
#include "arch/proc.h"
#include "server_map.h"

typedef struct {
    ServerMap server_map;
    int32_t fd[PROCS_MAX][MAX_FILES];
} Coordinator;

int32_t coordinator_update(int32_t fd, int32_t pid, int32_t type);
int32_t coordinator_noop(void);
int32_t coordinator_putchar(void);
int32_t coordinator_getchar(void);
int32_t coordinator_open(void);
int32_t coordinator_close(int32_t fd, int32_t pid);
int32_t coordinator_read(int32_t fd, int32_t pid);
int32_t coordinator_write(int32_t fd, int32_t pid);
int32_t coordinator_lseek(int32_t fd, int32_t pid);
int32_t coordinator_stat(void);
int32_t coordinator_dup(int32_t fd, int32_t pid);
int32_t coordinator_pipe(void);
int32_t coordinator_mkdir(void);
int32_t coordinator_rmdir(void);
int32_t coordinator_chdir(void);
int32_t coordinator_cwd(void);
int32_t coordinator_ls(void);
int32_t coordinator_mknod(void);
int32_t coordinator_link(void);
int32_t coordinator_unlink(void);
int32_t coordinator_chown(void);
int32_t coordinator_chmod(void);
void init_servers();
void init_coordinator();
#endif
