#include "stdio.h"
#include "types.h"
#include "coordinator.h"
#include "syscalls.h"


void main(int argc, char *argv[]) {

    init_coordinator();
    init_servers();
}