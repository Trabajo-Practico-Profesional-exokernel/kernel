#include "inc/syscalls.h"

#include "disk_syscalls.h" // Def of syscalls implemented here.
#include "lib.h" // For printf and syscall func

int sys_disk_read(size_t disk_pos, char* buffer, size_t read_len){
    return syscall(SYS_DISK_READ, (int) disk_pos, (int) buffer, (int) read_len);
}

int sys_disk_write(char* buffer, size_t disk_pos, size_t write_len){
    return syscall(SYS_DISK_WRITE, (int) buffer, (int) disk_pos, (int) write_len);
}
