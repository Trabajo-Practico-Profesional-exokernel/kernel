#include "pipe.h"
#include "files.h"
#include "buffer.h"
#include "constants.h"

#define TOTAL_FILES PROCS_MAX*MAX_FILES

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
    file_init(file_w, PERM_READ);

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
    return 0;
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

    return SUCCESS;
}

int32_t pipe_fstat(int32_t fd, int32_t app_id){
    return 0;
}
