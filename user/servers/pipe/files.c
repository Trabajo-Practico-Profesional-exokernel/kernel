#include "files.h"
#include "string.h"
#include "stdlib.h"
#include "arch/proc.h"
#include "constants.h"
#include "buffer.h"

#define TOTAL_FILES PROCS_MAX*MAX_FILES

struct File files[PROCS_MAX][MAX_FILES];

void init_files(){
    for (int i = 0; i < TOTAL_FILES; i++)
    {
        init_membuffers();
    }

    for (int i = 0; i < PROCS_MAX; i++)
    {
        for (int j = 0; j < MAX_FILES; j++)
        {
            file_reset(&files[i][j]);
        }  
    }
    
}

void file_retain(struct File*f){
    if (f->buffer == NULL) {
        return;
    }
    if (f->buffer->references < 255) {
        f->buffer->references++;
    }
}

struct File * alloc_file(int32_t pid){
    for (int i = 0; i < MAX_FILES; i++) 
    {
        if (files[pid][i].state == OFF) {
            file_reset(&files[pid][i]);
            return &files[pid][i];
        }
    }
    return NULL;
}

void file_reset(struct File *f) {
    memset(f, 0, sizeof(struct File));

    f->state = OFF;
    
    f->buffer = NULL;

    f->readopen = OFF;
    f->writeopen = OFF;
}


int file_write(struct File *f, const uint8_t *src, uint8_t len) {
    if (f->state == OFF) return ERROR;
    if (f->writeopen == OFF) return ERROR;

    int bytes_written = membuffer_write(f->buffer, src, len);

    return bytes_written;
}

int file_read(struct File *f, uint8_t *dst, uint8_t len) {
    if (f->state == OFF) return ERROR;
    if (f->readopen == OFF) return ERROR;

    int bytes_read = membuffer_read(f->buffer, dst, len);
    
    if (bytes_read == 0) {
        if (f->buffer->references > 1) {
            return -2;
        }
    }
    
    return bytes_read;
}

void file_close(struct File *f) {
    f->readopen = OFF;
    f->writeopen = OFF;
    f->state = OFF;

    if (f->buffer != NULL) {
        membuffer_release(f->buffer);
        f->buffer = NULL;
    }
}

void file_close_write(struct File *f) {
    f->writeopen = OFF;
    if (f->readopen == OFF) {
        file_close(f);
    }
}

void file_close_read(struct File *f) {
    f->readopen = OFF;
    if (f->writeopen == OFF) {
        file_close(f);
    }
}

int add_buffer_to_file(struct File *file, struct MemBuffer *buffer){
    if (file->buffer != NULL){
        return ERROR;
    }
    file->buffer = buffer;
    buffer->references++;
    return SUCCESS;
}

int get_fd(int32_t pid){
    for (int i = 0; i < MAX_FILES; i++) 
    {
        if (files[pid][i].state == OFF){
            return i;
        }
    }
    return ERROR;
}

void reset_fd(int32_t pid, int32_t fd){
    files[pid][fd].state = OFF;
}

void file_init(struct File *f, uint8_t perms) {
    
    f->state = ON;
    f->buffer = NULL;
    f->readopen = (perms & PERM_READ) ? ON : OFF;
    f->writeopen = (perms & PERM_WRITE) ? ON : OFF;
}

struct File * get_file(int32_t pid, int32_t fd){
    if (files[pid][fd].state == ON){
        return &files[pid][fd];
    }
    return NULL;
}

void asign_file_to_fd(int32_t fd, int32_t pid, struct File *file){

    *file = files[pid][fd];

}
