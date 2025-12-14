#ifndef KERNEL_FS_HANDLING
#define KERNEL_FS_HANDLING

#include "inc/types.h"

typedef void (*fs_event_handler)(int msg_count);

struct FilesystemEventsHandler {
    uintptr_t events_buffer;
    size_t buffer_len;
    
    fs_event_handler on_touch;
    fs_event_handler on_rm;
    fs_event_handler on_stat;
};

#endif