#ifndef INC_SLEEP_LOCKS
#define INC_SLEEP_LOCKS

#include "types.h"
#include "arch/spin_locks.h"

// Long-term locks for processes
struct sleeplock {
  uint8_t locked;     // Is the lock held?
  struct spinlock lk; // spinlock protecting this sleep lock
  
  // For debugging:
  char *name;        // Name of lock.
  int pid;           // Process holding lock
};

void acquiresleep(struct sleeplock*);
void releasesleep(struct sleeplock*);
int holdingsleep(struct sleeplock*);
void initsleeplock(struct sleeplock*, char*);

#endif /* !*/
