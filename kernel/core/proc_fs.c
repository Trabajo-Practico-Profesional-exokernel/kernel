#include "proc_fs.h"
#include "string.h"
#include "stdio.h"
#include "user_pages_alloc.h"
#include "arch_inc/mem_constants.h"
#include "sched.h"

static SystemInfo sys_info;

static const char* get_unix_status(int status) {
    switch (status) {
        case PROC_RUNNING:      return "R+";
        case PROC_RUNNABLE:     return "R";
        case PROC_NOT_RUNNABLE: return "S";
        case PROC_DYING:        return "Z";
        case PROC_FREE:         return "I";
        default:                return "?";
    }
}

int init_system_info(void) {
    memset(&sys_info, 0, sizeof(SystemInfo));

    //falta revisar el CPU count
    sys_info.kernel.cpu_count = 0; 

    sys_info.kernel.total_memory = user_pages_count() * PAGE_SIZE;
    sys_info.kernel.free_memory = sys_info.kernel.total_memory;

    return SUCCESS;
}

int update_system_info(int ticks, int use_of_cpu, int free_memory) {
    sys_info.kernel.ticks = ticks;
    sys_info.kernel.use_of_cpu = use_of_cpu;
    sys_info.kernel.free_memory = free_memory;
    return SUCCESS;
}

int add_proc_info(struct Proc *proc) {
    if (proc == NULL) {
        return ERROR;
    }

    int pid = proc->pid;

    if (pid < 0 || pid >= PROCS_MAX) {
        return ERROR;
    }

    ProcInfo *info = &sys_info.procs[pid];

    info->pid = proc->pid;
    info->gid = proc->gid;
    info->status = proc->status;
    info->cpunum = proc->cpunum;

    if (pid == coordinator_PID) {
        strcpy(info->proc_name, "coordinator");
    } else {
        strcpy(info->proc_name, proc->proc_name);
    }

    return SUCCESS;
}

int delete_proc_info(int proc_id) {
    if (proc_id < 0 || proc_id >= PROCS_MAX) {
        return ERROR;
    }

    memset(&sys_info.procs[proc_id], 0, sizeof(ProcInfo));
    sys_info.procs[proc_id].status = PROC_FREE;

    return SUCCESS;
}

int proc_ls(void) {

    int total_procs = 0;
    int running_procs = 0;
    int sleeping_procs = 0;
    int zombie_procs = 0;

    for (int i = 0; i < PROCS_MAX; i++) {
        if (sys_info.procs[i].status != PROC_FREE) {
            total_procs++;
            if (sys_info.procs[i].status == PROC_RUNNING || 
                sys_info.procs[i].status == PROC_RUNNABLE) running_procs++;
            else if (sys_info.procs[i].status == PROC_NOT_RUNNABLE) sleeping_procs++;
            else if (sys_info.procs[i].status == PROC_DYING) zombie_procs++;
        }
    }
    
    printf("top - uptime: %d ticks\n", sys_info.kernel.ticks);
    printf("Tasks: %d total, %d running, %d sleeping, %d zombie\n", 
            total_procs, running_procs, sleeping_procs, zombie_procs);
    
    printf("Cpu(s): %d%% user, %d%% idle\n", 
            sys_info.kernel.use_of_cpu, 
            100 - sys_info.kernel.use_of_cpu);
            
    int mem_total_kb = sys_info.kernel.total_memory / 1024;
    int mem_free_kb = sys_info.kernel.free_memory / 1024;
    int mem_used_kb = mem_total_kb - mem_free_kb;

    printf("MiB Mem : %d total, %d free, %d used\n", 
            mem_total_kb, mem_free_kb, mem_used_kb);
    
    printf("\n");

    printf("PID", "GID", "CPU", "STAT", "PROC\n");
    
    for (int i = 0; i < PROCS_MAX; i++) {
        ProcInfo *p = &sys_info.procs[i];

        if (p->status != PROC_FREE) {
            printf("%d %d %d %s %s\n",
                   p->pid,
                   p->gid,
                   p->cpunum,
                   get_unix_status(p->status),
                   p->proc_name);
        }
    }
    
    printf("\n");
    return 0;
}
