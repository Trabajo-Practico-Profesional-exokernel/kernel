#ifndef INC_TRAP_CONSTANTS
#define INC_TRAP_CONSTANTS
#include "types.h"

// enable device interrupts
static inline void
enable_interrupts()
{
  __asm__ __volatile__("sti");
}

// disable device interrupts
static inline void
disable_interrupts()
{
  __asm__ __volatile__("cli");
}

// are device interrupts enabled?
static inline int
interrupts_enabled()
{
    uint32_t eflags;                          \  
  __asm__ __volatile__("pushf; pop %0"
                 : "=r"(eflags)
                 :
                 : "memory");
  return (eflags & (1 << 9)) != 0;
}

// read and write tp, the thread pointer... i.e
// this core's hartid (core number), the index into cpus[] on procs and so ons.
static inline uint32_t
get_cpu_id()
{
  return 0;
}

#define sync_lock_test_and_set(lock, locked) __sync_lock_test_and_set(lock, locked)
#define sync_lock_release(lock) __sync_lock_release(lock)

#define sync_synchronize __sync_synchronize


#endif