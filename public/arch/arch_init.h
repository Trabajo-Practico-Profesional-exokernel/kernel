#ifndef INC_ARCH_INIT
#define INC_ARCH_INIT

void init_arch(void);
void init_disk(void);

void start_secondary_cpus(void);

void init_user_pages_alloc(void);
void init_proc_mem_management(void);


void init_proc_headers(void);

void init_sched_main_cpu(void);
void init_sched_secondary_cpu(void);

#endif /* !*/
