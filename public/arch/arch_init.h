#ifndef INC_ARCH_INIT
#define INC_ARCH_INIT

void init_arch(void);
void init_disk(void);

void notify_inited(void);

void init_user_pages_alloc(void);
void init_proc_mem_management(void);

#endif /* !*/
