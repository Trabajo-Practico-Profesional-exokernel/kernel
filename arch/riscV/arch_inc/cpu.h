#ifndef ARCH_INC_CPU
#define ARCH_INC_CPU

#include "inc/types.h"
#include "arch/proc.h"

// Saved registers for kernel context switches.
struct cpu {
  struct proc *proc; // The process running on this cpu, or null.
  int noff; // Depth of push_off() nesting. How many acquires from same proc or so?
  int intena; // Were interrupts enabled before push_off()?
};

#endif /* !*/




