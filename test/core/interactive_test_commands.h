#ifndef INTERACTIVE_TESTS_COMMANDS
#define INTERACTIVE_TESTS_COMMANDS

void init_cpus_commands(void);
void init_proc_commands(void);
void init_test_commands(void);
void init_irq_commands(void);
void init_info_commands(void);

void wait_start_cpus(void);
void add_start_cpu(void);
void wait_cpus_started(void);


#endif
