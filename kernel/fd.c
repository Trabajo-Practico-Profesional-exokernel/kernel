#include "arch/fd.h"
#include "fd.h"
#include "std/string.h"
#include "arch/proc.h"
#include "inc/common.h"

#define TOTAL_FILES PROCS_MAX*MAX_FILES

struct MemBuffer buffers[TOTAL_FILES];
struct File files[TOTAL_FILES];

void init_files(){
    for (int i = 0; i < TOTAL_FILES; i++)
    {
        membuffer_reset(&buffers[i]);
        fd_reset(&files[i]);
    }
}

void membuffer_reset(struct MemBuffer *buffer){
    memset(buffer, 0, sizeof(struct MemBuffer));
}

struct MemBuffer* membuffer_alloc(void) {
    for (int i = 0; i < TOTAL_FILES; i++) {
        if (buffers[i].references == 0) {
            memset(&buffers[i], 0, sizeof(struct MemBuffer));
            buffers[i].references = 1;
            return &buffers[i];
        }
    }
    PANIC("No free MemBuffer in membuffer_alloc");
}


void membuffer_release(struct MemBuffer *mb) {
    if (mb == NULL){
        return;
    }

    if (mb->references > 0) {
        mb->references--;
    }

    if (mb->references == 0) {
        memset(mb, 0, sizeof(struct MemBuffer));
    }
}

void membuffer_retain(struct MemBuffer *mb) {
    if (mb == NULL) {
        return;
    }
    if (mb->references < 255) {
        mb->references++;
    }
}

int membuffer_write(struct MemBuffer *mb, const uint8_t *src, uint8_t len) {
    if (mb == NULL) {
        return -1;
    }
    
    int bytes_written = 0;

    while (bytes_written < len) {

        if (mb->len_buffer >= MAX_BUFFER_BYTES) {
            break; 
        }

        mb->buffer[mb->write_idx] = src[bytes_written];

        mb->write_idx = (mb->write_idx + 1) % MAX_BUFFER_BYTES;
        
        mb->len_buffer++;
        bytes_written++;
    }

    return bytes_written;
}

int membuffer_read(struct MemBuffer *mb, uint8_t *dst, uint8_t len) {
    if (mb == NULL) {
        return -1;
    }

    int bytes_read = 0;

    while (bytes_read < len) {
        if (mb->len_buffer == 0) {
            break; 
        }

        dst[bytes_read] = mb->buffer[mb->read_idx];

        mb->read_idx = (mb->read_idx + 1) % MAX_BUFFER_BYTES;
        mb->len_buffer--;
        bytes_read++;
    }

    return bytes_read;
}


int membuffer_is_full(struct MemBuffer *mb) {
    if (mb == NULL) {
        return 0;
    }
    return (mb->len_buffer >= MAX_BUFFER_BYTES);
}

int membuffer_is_empty(struct MemBuffer *mb) {
    if (mb == NULL) {
        return 1;
    }

    return (mb->len_buffer == 0);
}


struct File * alloc_file(){
    for (int i = 0; i < TOTAL_FILES; i++)
    {
        if (files[i].state == OFF) {
            fd_reset(&files[i]);
            return &files[i];
        }
    }

    PANIC("No free file in alloc_file");
}

void fd_reset(struct File *f) {
    memset(f, 0, sizeof(struct File));

    f->state = OFF;
    f->type = FD_TYPE_NONE;
    f->perms = 0;
    
    f->buffer = NULL;

    f->readopen = OFF;
    f->writeopen = OFF;
}

void fd_init(struct File *f, fd_type_t type, uint8_t perms) {
    f->state = ON;
    f->type = type;
    f->perms = perms;
    
    f->buffer = NULL;

    f->readopen = (perms & PERM_READ) ? ON : OFF;
    f->writeopen = (perms & PERM_WRITE) ? ON : OFF;
}


int fd_write(struct File *f, const uint8_t *src, uint8_t len) {
    if (f->state == OFF) return E_CLOSED;
    if (!(f->perms & PERM_WRITE)) return E_PERM;
    if (f->writeopen == OFF) return E_CLOSED;

    int bytes_written = membuffer_write(f->buffer, src, len);

    return bytes_written;
}

int fd_read(struct File *f, uint8_t *dst, uint8_t len) {
    if (f->state == OFF) return E_CLOSED;
    if (!(f->perms & PERM_READ)) return E_PERM;
    if (f->readopen == OFF) return E_CLOSED;

    int bytes_read = membuffer_read(f->buffer, dst, len);

    return bytes_read;
}

void fd_close(struct File *f) {
    f->readopen = OFF;
    f->writeopen = OFF;
    f->state = OFF;
    f->type = FD_TYPE_NONE;
    if (f->buffer != NULL) {
        membuffer_release(f->buffer);
        f->buffer = NULL;
    }
}

void fd_close_write(struct File *f) {
    f->writeopen = OFF;
    if (f->readopen == OFF) {
        fd_close(f);
    }
}

void fd_close_read(struct File *f) {
    f->readopen = OFF;
    if (f->writeopen == OFF) {
        fd_close(f);
    }
}
