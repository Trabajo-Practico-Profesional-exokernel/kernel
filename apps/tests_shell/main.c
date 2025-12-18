#include "lib.h"
#include "std/string.h"

// Own includes
#include "command_handler.h"

void main() {
    printf("Test start/exec proc_b\n");
    exec_command("exec", "proc_b");

    printf("Test start/exec proc_a\n");
    exec_command("exec", "proc_a");

    printf("Test start/exec hello_world\n");
    exec_command("exec", "hello_world");

    printf("Test start/exec periodic_yield\n");
    exec_command("exec", "periodic_yield");
    
}