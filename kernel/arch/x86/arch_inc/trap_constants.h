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

static inline void
enable_timer_interrupts()
{
	enable_interrupts();
}

static inline void
disable_timer_interrupts()
{
	disable_interrupts();
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


#define sync_lock_test_and_set(lock, locked) __sync_lock_test_and_set(lock, locked)
#define sync_lock_release(lock) __sync_lock_release(lock)

#define sync_synchronize __sync_synchronize


#endif
