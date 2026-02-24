#include "lib.h"
#include "syscalls.h"
#include "string.h"
#include "stdlib.h"
#include "environ.h"

void main(int argc, char ** argv) {
    printf("========ENV PROCESS \n");
    char ** env_list = get_environ_list();
    size_t count = 0;
    while (env_list[count]){
        printf("env at: %d '%s'\n",count, env_list[count]); 
        count++;
    }

    printf("env got %d vars\n",count); 
}