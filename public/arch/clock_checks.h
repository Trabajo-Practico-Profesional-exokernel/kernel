#ifndef CLOCK_CHECKS
#define CLOCK_CHECKS

#include "arch/proc.h"

void add_uptime_to_proc(struct Proc * curr_proc);
void add_idle_time(struct Proc * idle_proc);
void check_sleeping_proc(void);
// void check_io_locked_proc(void);
// void check_stdin_locked_proc(void);
// void check_ipc_locked_proc(void);

#endif