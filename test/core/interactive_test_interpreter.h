#ifndef INTERACTIVE_TESTS
#define INTERACTIVE_TESTS

void init_interactive_tests(void);
void interactive_shell_main(void);
void interactive_shell_help(void);

#include "arch/proc.h"

void resume_interactive_shell(struct Proc * last_proc);

#endif