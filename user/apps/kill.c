#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "parsers/strutil.h"

void main(int argc, char ** argv) {
    if (argc < 2) {
        printf("Usage: kill [-p] <id>\n If -p not passed id is gid, else pid\n");
        exit(1);
    }


    if (strncmp((const uint8_t*)argv[1], (const uint8_t*)"-p", 3) == 0) {
        if (argc < 3) {
            printf("Usage: kill [-p] <pid>\n Did not have <pid>\b");
            exit(1);
        }
        
        uint32_t pid = atoi((const uint8_t*)argv[2]);
        int killed_count = sys_kill(pid);
        
        if (killed_count > 0) {
            printf("kill: proc killed! %u count: %d \n", pid, killed_count);
            exit(0);
        } else {
            printf("kill: could not kill %u\n", pid);
            exit(1);
        }
    }
    
    int gid = atoi((const uint8_t*)argv[1]);
    
    int killed_count = sys_kill_group(gid);

    if (killed_count > 0) {
        printf("kill: killed! gid: %u count: %d \n", gid, killed_count);
        exit(0);
    } else {
        printf("kill: could not kill gid %u\n", gid);
        exit(1);
    }
    


}