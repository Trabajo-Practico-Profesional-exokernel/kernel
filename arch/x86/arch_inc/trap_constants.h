#ifndef INC_TRAP_CONSTANTS
#define INC_TRAP_CONSTANTS
#include "inc/types.h"

// enable device interrupts
static inline void
enable_interrupts()
{
}

// disable device interrupts
static inline void
disable_interrupts()
{
}

// are device interrupts enabled?
static inline int
interrupts_enabled()
{
  return true;
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