#include "arch/arch_init.h"
#include "arch/trap_handling.h"
#include "arch/mem.h"
#include "arch/proc.h"

#include "inc/common.h"
#include "std/string.h"

#include "arch_inc/virtio_blk.h"
#include "arch_inc/virtio.h"

extern void sched_yield(void);
extern void save_curr_proc_state(FullTrapFrame *tf, uintptr_t pc);
extern struct Proc * get_curr(void);


void 
sys_ide_r(FullTrapFrame *tf, uintptr_t pc)
{

}

void
sys_ide_w(FullTrapFrame *tf, uintptr_t pc)
{
}

