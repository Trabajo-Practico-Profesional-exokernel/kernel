
#ifndef PROC_FS_H
#define PROC_FS_H

#include "arch/proc.h"
#include "types.h"
#include "constants.h"

typedef struct{

    int ticks;
    int use_of_cpu;
    int cpu_count;
    int free_memory;
    int total_memory;

} KernelInfo;

typedef struct{

    char proc_name[MAX_NAME];
    procid_t pid;
    procid_t gid;
    int status;
    int cpunum;

} ProcInfo;

typedef struct{
    KernelInfo kernel;
    ProcInfo procs[PROCS_MAX];
} SystemInfo;

void init_system_info();
void update_system_info(int ticks, int use_of_cpu);
void update_system_memory(int memory);
int delete_proc_info(int proc_id);
int proc_ls();


int add_proc_info(struct Proc *proc);
int upd_proc_gid(struct Proc *proc);

void add_proc_to_system_stats(struct Proc* proc);

#endif
