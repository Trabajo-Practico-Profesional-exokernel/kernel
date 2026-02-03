#ifndef CONSOLE_FILES_H
#define CONSOLE_FILES_H

#include "types.h"

#define STDIN 0
#define STDOUT 1
#define MAX_IO_FILES 2


typedef enum {
    IO_NONE = 0,
    IO_READ_ONLY,
    IO_WRITE_ONLY
} io_type_t;

struct ConsoleFile {
    uint8_t is_open;
    io_type_t type;
};

void reset_std_files();
void init_proc_std_files(int pid);
int console_file_close(int32_t pid, int32_t fd);
int console_file_write(int32_t pid, int32_t fd, const char *buf, int len);
int console_file_read(int32_t pid, int32_t fd, const char *buf, int len);

#endif
