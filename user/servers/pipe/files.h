#ifndef FILES_H
#define FILES_H

#include "types.h"

typedef enum {
    OFF = 0,
    ON  = 1
} file_state_t;

typedef enum {
    PERM_WRITE = 1,
    PERM_READ  = 2
} file_perms;


struct File {
    file_state_t state; //estado actual del file que marca si esta siendo usado o no

    struct MemBuffer *buffer;

    file_state_t readopen; //estado extremo escritura
    file_state_t writeopen; //estado extremo lectura
};

void init_files();

void file_reset(struct File *f);

void file_init(struct File *f, uint8_t perms);

int file_write(struct File *f, const uint8_t *src, uint8_t len);

int file_read(struct File *f, uint8_t *dst, uint8_t len);

void file_close(struct File *f);

int file_can_read(struct File *f);

int file_can_write(struct File *f);

void file_retain(struct File *f);

int get_file_descriptor(int32_t id);

struct File * alloc_file(int32_t pid);

void file_close_write(struct File *f);

void file_close_read(struct File *f);
void file_retain(struct File*f);
struct File * get_file(int32_t pid, int32_t fd);
void asign_file_to_fd(int32_t fd, int32_t pid, struct File *file);
int get_fd(int32_t pid);
int add_buffer_to_file(struct File *file, struct MemBuffer *buffer);
#endif