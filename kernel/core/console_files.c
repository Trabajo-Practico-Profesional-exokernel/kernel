
#include "console_files.h"
#include "arch/proc.h"
#include "arch/console.h"
#include "constants.h"

struct ConsoleFile console_files[PROCS_MAX][2];

void console_file_init(struct ConsoleFile *f, io_type_t type) {
    f->is_open = 1;
    f->type = type;
}

void console_file_reset(struct ConsoleFile *f) {
    f->is_open = 0;
    f->type = IO_NONE;
}

int console_file_close(int32_t pid, int32_t fd) {

    if (pid < 0 || pid >= PROCS_MAX) return ERROR;
    if (fd < 0 || fd > MAX_IO_FILES) return ERROR;
    console_files[pid][fd].is_open = 0;
    console_files[pid][fd].type = IO_NONE;

    return SUCCESS;
}


void init_proc_std_files(int pid){
    console_file_init(&console_files[pid][STDIN], IO_READ_ONLY);
    console_file_init(&console_files[pid][STDOUT], IO_WRITE_ONLY);
}

void reset_std_files() {

    for(int i=0; i<PROCS_MAX; i++){
        console_file_reset(&console_files[i][STDIN]);
        console_file_reset(&console_files[i][STDOUT]);
    }
    
}

int console_file_write(int32_t pid, int32_t fd, const char *buf, int len) {

    if (pid < 0 || pid >= PROCS_MAX) return -1;
    if (fd < 0 || fd > MAX_IO_FILES) return -1;
    if (console_files[pid][fd].is_open == 0) return -1;
    if (console_files[pid][fd].type!= IO_WRITE_ONLY) return -1;

    int written = console_write(buf, len);

    return written;
}

int console_file_read(int32_t pid, int32_t fd, const char *buf, int len) {
    
    if (pid < 0 || pid >= PROCS_MAX) return -1;
    if (fd < 0 || fd > MAX_IO_FILES) return -1;
    if (console_files[pid][fd].is_open == 0) return -1;
    if (console_files[pid][fd].type!= IO_READ_ONLY) return -1;

    int read = console_read(buf, len);

    return read;
}
