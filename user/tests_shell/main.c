#include "lib.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "console/debug.h"

// Own includes
#include "command_handler.h"

void main() {
    debug_printf("Test start/exec proc_b\n");
    exec_command("exec", "proc_b");

    debug_printf("Test start/exec proc_a\n");
    exec_command("exec", "proc_a");

    debug_printf("Test start/exec hello_world\n");
    exec_command("exec", "hello_world");

    debug_printf("Test start/exec periodic_yield\n");
    exec_command("exec", "periodic_yield");
    
}