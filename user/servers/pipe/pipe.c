#include "pipe.h"
#include "files.h"
#include "buffer.h"
#include "constants.h"

#define TOTAL_FILES PROCS_MAX*MAX_FILES

extern struct File files[PROCS_MAX][MAX_FILES];

void pipe_init(void){
    init_files();
}

int32_t pipe_open(int32_t app_id, int32_t *fds){

    struct MemBuffer* buffer = membuffer_alloc();
    membuffer_reset(buffer);

    int fd_r = get_fd(app_id);

    if (fd_r == ERROR){
        return ERROR;
    }

    struct File *file_r = alloc_file(app_id);
    file_init(file_r, PERM_READ);

    int fd_w = get_fd(app_id);

    if (fd_w == ERROR){
        return ERROR;
    }

    struct File *file_w = alloc_file(app_id);
    file_init(file_w, PERM_WRITE);

    add_buffer_to_file(file_r, buffer);
    add_buffer_to_file(file_w, buffer);

    fds[0] = fd_r;
    fds[1] = fd_w;

    return SUCCESS;
}

int32_t pipe_read(int32_t fd, int32_t app_id, char *buf, int32_t len){
    
    struct File *file = get_file(app_id, fd);

    if (file == NULL){
        return ERROR;
    }

    return file_read(file, buf, len);

}

int32_t pipe_write(int32_t fd, int32_t app_id, char *buf, int32_t len){
    
    struct File *file = get_file(app_id, fd);

    if (file == NULL){
        return ERROR;
    }

    return file_write(file, buf, len);
}

int32_t pipe_close(int32_t fd, int32_t app_id){

    struct File *file = get_file(app_id, fd);

    if (file == NULL){
        return ERROR;
    }

    file_close(file);
    reset_fd(app_id, fd);

    return SUCCESS;
}

int32_t pipe_dup(int32_t fd, int32_t app_id){

    int dup_fd = get_fd(app_id);

    if (dup_fd == ERROR){
        return ERROR;
    }

    struct File *file = alloc_file(app_id);

    if (file == NULL){
        return ERROR;
    }

    asign_file_to_fd(fd, app_id, file);
    file_retain(file);

    return dup_fd;
}

int32_t pipe_fstat(int32_t fd, int32_t app_id){
    return 0;
}

int32_t pipe_fork(int32_t app_pid, int32_t app_father) {
    if (app_pid < 0 || app_pid >= PROCS_MAX || app_father < 0 || app_father >= PROCS_MAX) {
        return ERROR;
    }
    // printf("[PIPE] pipe_fork: Replicando estado de Padre [%d] a Hijo [%d]\n", app_father, app_pid);
    
    for (int i = 0; i < MAX_FILES; i++) {
        struct File *father_file = get_file(app_father, i);
        if (father_file != NULL) {
            // printf("[PIPE] pipe_fork: Hijo [%d] asume FD [%d]\n", app_pid, i);
            files[app_pid][i].state = father_file->state;
            files[app_pid][i].readopen = father_file->readopen;
            files[app_pid][i].writeopen = father_file->writeopen;
            files[app_pid][i].buffer = father_file->buffer;
            
            file_retain(&files[app_pid][i]);
        }
    }
    return SUCCESS;
}

int32_t pipe_close_all(int32_t app_pid) {
    if (app_pid < 0 || app_pid >= PROCS_MAX) {
        return ERROR;
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (files[app_pid][i].state == ON) {
            pipe_close(i, app_pid);
        }
    }
    return SUCCESS;
}