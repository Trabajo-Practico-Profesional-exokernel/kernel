#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "parsers/strutil.h"

void main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Usage: kill <pid>\n");
        exit(1);
    }
    uint32_t pid = atoi((const uint8_t*)argv[1]);
    
    if (sys_kill(pid) == 0) {
		printf("kill: killed! %u \n", pid);
        exit(0);
    } else {
        printf("kill: cannot kill %u\n", pid);
        exit(1);
    }
}