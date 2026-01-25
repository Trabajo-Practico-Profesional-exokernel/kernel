#ifndef DEFAULT_EXECUTABLES
#define DEFAULT_EXECUTABLES
#include "types.h"

bool default_executable_check(char* executable_name, char*args, int* out_ret);
int exec_program(char* program_name, char*args);

#define ERR_CODE -1
#define OK_CODE 0

#endif
