#ifndef EXPOSED_DISK_H
#define EXPOSED_DISK_H

#include "types.h"

int read_disk(void *buf, int offset, int length);


// Sets the offset at which syscalls will start reading/writing for user procs
// Before this offset is reserved for the app headers/programs and kernel use.
void set_user_fs_start(int bytes_offset);

#endif /* !*/


