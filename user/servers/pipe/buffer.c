#include "string.h"
#include "stdlib.h"
#include "arch/proc.h"
#include "constants.h"
#include "buffer.h"
#include "arch/proc.h"

#define TOTAL_FILES PROCS_MAX*MAX_FILES

struct MemBuffer buffers[TOTAL_FILES];

void init_membuffers(){
    for (int i = 0; i < TOTAL_FILES; i++)
    {
        membuffer_reset(&buffers[i]);
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

int membuffer_write(struct MemBuffer *mb, const uint8_t *src, uint8_t len) {
    if (mb == NULL) {
        return -1;
    }
    
    int bytes_written = 0;

    while (bytes_written < len) {
        if (mb->len_buffer >= MAX_BUFFER_BYTES) {
            break; 
        }
        int write_index = mb->write_idx;
        mb->buffer[write_index] = src[bytes_written];
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
