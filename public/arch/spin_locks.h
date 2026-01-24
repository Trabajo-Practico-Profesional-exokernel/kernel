#ifndef INC_SPIN_LOCKS
#define INC_SPIN_LOCKS

#include "inc/types.h"

// Mutual exclusion lock.
struct spinlock {
  uint8_t locked;  // Is the lock held?
  // For debugging:
  char *name;      // Name of lock.
  int cpuid;       // The cpu holding the lock.
};

void acquire(struct spinlock*);
int holding(struct spinlock*);
void initlock(struct spinlock*, char*);
void release(struct spinlock*);
void push_off(void);
void pop_off(void);

#endif /* !*/
