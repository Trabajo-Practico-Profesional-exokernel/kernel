// Mutual exclusion spin locks.

#include "arch/spin_locks.h"
#include "arch_inc/trap_constants.h"
#include "arch/cpus.h"
#include "constants.h"
#include "console/debug.h"

void
initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpuid = -1;
}


int try_acquire(struct spinlock* lk){
  push_off(); // disable interrupts to avoid deadlock.
  if(holding(lk))
    PANIC("acquire already holding?");

  if(sync_lock_test_and_set(&lk->locked, 1) != 0){
    pop_off();
    return -1;
  }
  
  sync_synchronize();
  lk->cpuid = cpuid();
  return 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
void
acquire(struct spinlock *lk)
{
  push_off(); // disable interrupts to avoid deadlock.
  if(holding(lk))
    PANIC("acquire already holding?");

  // On RISC-V, sync_lock_test_and_set turns into an atomic swap:
  //   a5 = 1
  //   s1 = &lk->locked
  //   amoswap.w.aq a5, a5, (s1)
  while(sync_lock_test_and_set(&lk->locked, 1) != 0)
    ;
  // printf("CPU UNLOCKED! cpuid %d locked? %d\n", cpuid(), lk->locked);

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen strictly after the lock is acquired.
  // On RISC-V, this emits a fence instruction.
  sync_synchronize();

  // Record info about lock acquisition for holding() and debugging.
  lk->cpuid = cpuid();
  // printf("NEW CPU ACQUIRED LOCK! %d\n", lk->cpuid);
}

// Release the lock.
void
release(struct spinlock *lk)
{
  if(!holding(lk)){
    printf("Curr cpu %d !== lock cpu %d\n", cpuid(), lk->cpuid);
    PANIC("release did not hold?");
    
  }
  // printf("CPU RELEASE LOCK! %d\n", lk->cpuid);

  lk->cpuid = -1;

  // Tell the C compiler and the CPU to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other CPUs before the lock is released,
  // and that loads in the critical section occur strictly before
  // the lock is released.
  // On RISC-V, this emits a fence instruction.
  sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code doesn't use a C assignment, since the C standard
  // implies that an assignment might be implemented with
  // multiple store instructions.
  // On RISC-V, sync_lock_release turns into an atomic swap:
  //   s1 = &lk->locked
  //   amoswap.w zero, zero, (s1)

  sync_lock_release(&lk->locked);

  pop_off();
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int
holding(struct spinlock *lk)
{
  int r;
  r = (lk->locked && lk->cpuid == cpuid());
  return r;
}

// push_off/pop_off are like intr_off()/intr_on() except that they are matched:
// it takes two pop_off()s to undo two push_off()s.  Also, if interrupts
// are initially off, then push_off, pop_off leaves them off.

void
push_off(void)
{
  int old = interrupts_enabled();

  // disable interrupts to prevent an involuntary context
  // switch while using mycpu().
  disable_interrupts();

  struct cpu* currcpu = mycpu();
  if(currcpu->noff == 0)
    currcpu->intena = old;
  currcpu->noff += 1;
}

void
pop_off(void)
{
  struct cpu *c = mycpu();
  if(interrupts_enabled())
    PANIC("pop_off - interruptible");
  
  if(c->noff < 1)
    PANIC("pop_off");
  c->noff -= 1;
  if(c->noff == 0 && c->intena)
    enable_interrupts();
}
