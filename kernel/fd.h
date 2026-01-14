#ifndef FD_H
#define FD_H

#include "arch/fd.h"

#define MAX_BYTES 255
#define PERM_READ  0x01
#define PERM_WRITE 0x02

#define E_OK       0
#define E_FULL    -1
#define E_EMPTY   -2
#define E_PERM    -3
#define E_CLOSED  -4
struct Proc;
void init_files();

void fd_reset(struct File *f);

void fd_init(struct File *f, fd_type_t type, uint8_t perms);

int fd_write(struct File *f, const uint8_t *src, uint8_t len);

int fd_read(struct File *f, uint8_t *dst, uint8_t len);

void fd_close(struct File *f);

void fd_close_write(struct File *f);

void fd_close_read(struct File *f);

int fd_can_read(struct File *f);

int fd_can_write(struct File *f);

void membuffer_reset(struct MemBuffer *buffer);

struct MemBuffer* membuffer_alloc(void);

void membuffer_release(struct MemBuffer *mb);

void fd_retain(struct File *f);

int membuffer_write(struct MemBuffer *mb, const uint8_t *src, uint8_t len);

int membuffer_read(struct MemBuffer *mb, uint8_t *dst, uint8_t len);

int membuffer_is_full(struct MemBuffer *mb);

int membuffer_is_empty(struct MemBuffer *mb);

int add_buffer_to_file(struct File *file, struct MemBuffer *buffer);

int add_buffer_to_file(struct File *file, struct MemBuffer *buffer);

int get_file_descriptor(struct Proc *proc);

struct File * alloc_file();
#endif /* FS_SYSCALLS_H */