#include "inc/syscalls.h"

#include "syscalls.h" // Def of syscalls implemented here.
#include "lib.h" // For printf and syscall func

int exec(int prog_ind, char ** args){
    // convert args pointer to int
    return syscall(SYS_EXEC, prog_ind, (int)(args), 0);
}


void putchar(char ch) {
    syscall(SYS_PUTCHAR, ch, 0, 0);
}

int getchar(void) {
    return syscall(SYS_GETCHAR, 0, 0, 0);
}
