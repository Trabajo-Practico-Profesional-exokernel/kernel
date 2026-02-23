#include "lib.h"
#include "environ.h"

void setup_from_env(void){
    char *path = getenv("pwd");
    if(path && *path != 0 && chdir(path) != 0) {
        printf("Failed chdir pwd\n");
        exit(2);
    }

}
