#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"


#define STDIN 0
#define STDOUT 1

#define CONSOLE_BUFFER_SIZE 256
#define MAX_WAITING_PROCS 10

struct Console {
    char input_buf[CONSOLE_BUFFER_SIZE]; 
    uint32_t read_idx;
    uint32_t write_idx;
    uint32_t count;

    int readers[MAX_WAITING_PROCS];
    int readers_count;
};

void console_init();

int32_t console_read(char *buf, int size);

int32_t console_write(char *buf, int size);

int console_push_input(uint8_t c);

int console_add_waiter(int pid);

int console_release_waiter_pid();

#endif
