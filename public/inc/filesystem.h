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

#endif