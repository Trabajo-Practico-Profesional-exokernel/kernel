#include "arch/fd.h"

#define MAX_BYTES 255
#define PERM_READ  0x01
#define PERM_WRITE 0x02

#define E_OK       0
#define E_FULL    -1
#define E_EMPTY   -2
#define E_PERM    -3
#define E_CLOSED  -4

void fd_init(struct File *f, fd_type_t type, uint8_t perms) {
    f->state = ON;
    f->type = type;
    f->perms = perms;
    
    f->len_buffer = 0;
    f->index_readed = 0;
    f->index_readedwrited = 0;

    f->readopen = (perms & PERM_READ) ? ON : OFF;
    f->writeopen = (perms & PERM_WRITE) ? ON : OFF;
}

int fd_write(struct File *f, const uint8_t *src, uint8_t len) {
    if (f->state == OFF) return E_CLOSED;
    if (!(f->perms & PERM_WRITE)) return E_PERM;
    if (f->writeopen == OFF) return E_CLOSED;

    int bytes_written = 0;

    while (bytes_written < len) {
        if (f->len_buffer >= MAX_BYTES) {
            break; 
        }

        f->buffer[f->index_readedwrited] = src[bytes_written];

        f->index_readedwrited = (f->index_readedwrited + 1) % MAX_BYTES;
        
        f->len_buffer++;
        bytes_written++;
    }

    return bytes_written;
}

int fd_read(struct File *f, uint8_t *dst, uint8_t len) {
    if (f->state == OFF) return E_CLOSED;
    if (!(f->perms & PERM_READ)) return E_PERM;
    
    if (f->len_buffer == 0 && f->writeopen == OFF) return 0;

    int bytes_read = 0;

    while (bytes_read < len) {
        if (f->len_buffer == 0) {
            break; 
        }

        dst[bytes_read] = f->buffer[f->index_readed];

        f->index_readed = (f->index_readed + 1) % MAX_BYTES;

        f->len_buffer--;
        bytes_read++;
    }

    return bytes_read;
}

void fd_close(struct File *f) {
    f->readopen = OFF;
    f->writeopen = OFF;
    f->state = OFF;
    f->type = FD_TYPE_NONE;
}

void fd_close_write(struct File *f) {
    f->writeopen = OFF;
    if (f->readopen == OFF) {
        f->state = OFF;
    }
}

void fd_close_read(struct File *f) {
    f->readopen = OFF;
    if (f->writeopen == OFF) {
        f->state = OFF;
    }
}

int fd_can_read(struct File *f) {
    return (f->len_buffer > 0);
}

int fd_can_write(struct File *f) {
    return (f->len_buffer < MAX_BYTES);
}
