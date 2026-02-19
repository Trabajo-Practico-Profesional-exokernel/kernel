#ifndef INTERACTIVE_TESTS_COMMANDS
#define INTERACTIVE_TESTS_COMMANDS

void start_cpus(void);
void wait_start_cpus(void);
void wait_cpus_started(void);
void add_start_cpu(void);



void load_processes_headers(void);
void start_shell(void);
void run_tests(void);

void do_sched_yield(char * args);
int handle_create_proc(char*program_name);


#endif
