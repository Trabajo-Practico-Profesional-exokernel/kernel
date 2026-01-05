#ifndef DISK_SYSCALLS_H
#define DISK_SYSCALLS_H


#include "inc/filesystem.h"


int disk_read(size_t disk_pos, char* buffer, size_t read_len);
int disk_write(char* buffer, size_t disk_pos, size_t write_len);

#endif
