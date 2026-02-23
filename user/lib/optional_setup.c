#include "lib.h"
#include "environ.h"

void setup_from_env(void){
    char *path = getenv("pwd");
    if(path && chdir(path) != 0) {
        printf("Failed chdir pwd");
        exit(2);
    }

}
