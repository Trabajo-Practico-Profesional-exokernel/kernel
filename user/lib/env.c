#include "environ.h"

static char **environ;

void init_environ(char ** envp){
	environ = envp;
}
