#ifndef pipe_H
#define pipe_H

#include "types.h"
#include "files.h"
#include "arch/proc.h"


void pipe_init(void);

int32_t pipe_open(int32_t app_id, int32_t *fds);

int32_t pipe_read(int32_t fd, int32_t app_id, char *buf, int32_t len);

int32_t pipe_write(int32_t fd, int32_t app_id, char *buf, int32_t len);

int32_t pipe_close(int32_t fd, int32_t app_id);

int32_t pipe_dup(int32_t fd, int32_t app_id);

int32_t pipe_fstat(int32_t fd, int32_t app_id);

int32_t pipe_fork(int32_t app_pid, int32_t app_father);

int32_t pipe_close_all(int32_t app_pid);


#endif
