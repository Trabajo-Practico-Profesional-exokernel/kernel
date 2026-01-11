#ifndef PUBLIC_ARCH_FILESYSTEM_H
#define PUBLIC_ARCH_FILESYSTEM_H

#include "inc/types.h"

typedef void (*fs_event_handler)(int msg_count);

struct FilesystemEventsHandler {
    void * buffer;
    size_t buffer_len;
    
    fs_event_handler on_touch;
    fs_event_handler on_stat;
    fs_event_handler on_rm;
};

struct stat {
    short type; //Tipo de archivo
    int dev; // ID dispositivo
    uint32_t size; //tamaño en bytes
};

// Estructuras auxiliares
typedef struct {
    int fd;
    int count;
    char data[0];
} fs_rw_req_t;

typedef struct {
    int fd;
    int offset;
} fs_seek_req_t;

#endif