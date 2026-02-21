#ifndef STATUS_LOGGING_H
#define STATUS_LOGGING_H

void info_procs_with_state(int state);
void info_procs_not_free(void);


void info_proc_exit_status(int pid);
void info_proc_uptime(int pid);
void info_proc_memory(int pid);


void info_uptimes(void);
void info_user_mem(void);


#endif