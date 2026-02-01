#ifndef INC_TRAP_CONSTANTS
#define INC_TRAP_CONSTANTS
#include "types.h"

#define MIE_STIE (1L << 5)  // supervisor timer
#define SIE_STIE (1L << 5)  // supervisor timer in S-Mode

#define SSTATUS_SPP (1L << 8)  // Previous mode, 1=Supervisor, 0=User
#define SSTATUS_SPIE (1L << 5) // Supervisor Previous Interrupt Enable
#define SSTATUS_UPIE (1L << 4) // User Previous Interrupt Enable
#define SSTATUS_SIE (1L << 1)  // Supervisor Interrupt Enable
#define SSTATUS_UIE (1L << 0)  // User Interrupt Enable


static inline uint32_t
r_sie(void)
{
    uint32_t x;
    __asm__ __volatile__("csrr %0, sie" : "=r"(x));
    return x;
}

static inline void
w_sie(uint32_t x)
{
    __asm__ __volatile__("csrw sie, %0" :: "r"(x));
}


// riscv specific
#define READ_CSR(reg)                                                          \
    ({                                                                         \
        uintptr_t __tmp;                                                   \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                  \
        __tmp;                                                                 \
    })

#define WRITE_CSR(reg, value)                                                  \
    do {                                                                       \
        uintptr_t __tmp = (value);                                              \
        __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                \
    } while (0)




static inline void 
w_stimecmp(uint32_t x)
{
  // __asm__ __volatile__("csrw stimecmp, %0" : : "r" (x));
  __asm__ __volatile__("csrw 0x14d, %0" : : "r" (x));
}


// machine-mode cycle counter
static inline uint32_t
r_time()
{
  uint32_t x;
  __asm__ __volatile__("csrr %0, time" : "=r" (x) );
  return x;
}




static inline uint32_t
r_sstatus()
{
  uint32_t x;
  __asm__ __volatile__("csrr %0, sstatus" : "=r" (x) );
  return x;
}

static inline void 
w_sstatus(uint32_t x)
{
  __asm__ __volatile__("csrw sstatus, %0" : : "r" (x));
}


// enable device interrupts
static inline void
enable_interrupts()
{
  w_sstatus(r_sstatus() | SSTATUS_SIE);
}

// disable device interrupts
static inline void
disable_interrupts()
{
  w_sstatus(r_sstatus() & ~SSTATUS_SIE);
}

// are device interrupts enabled?
static inline int
interrupts_enabled()
{
  uint32_t x = r_sstatus();
  return (x & SSTATUS_SIE) != 0;
}

// disable supervisor timer interrupts only
static inline void
disable_timer_interrupts(void)
{
    w_sie(r_sie() & ~SIE_STIE);
}

static inline void
enable_timer_interrupts(void)
{
    w_sie(r_sie() | SIE_STIE);
}



// read and write tp, the thread pointer... i.e
// this core's hartid (core number), the index into cpus[] on procs and so ons.
static inline uint32_t
get_cpu_id()
{
  uint32_t x;
  __asm__ __volatile__("mv %0, tp" : "=r" (x) );
  return x;
}


#define sync_lock_test_and_set(lock, locked) __sync_lock_test_and_set(lock, locked)
#define sync_lock_release(lock) __sync_lock_release(lock)

#define sync_synchronize __sync_synchronize


static inline uint32_t
r_mhartid()
{
  uint32_t x;
  __asm__ __volatile__("csrr %0, mhartid" : "=r" (x) );
  return x;
}

static inline uint32_t
r_tp()
{
  uint32_t x;
  __asm__ __volatile__("mv %0, tp" : "=r" (x) );
  return x;
}

static inline void 
w_tp(uint32_t x)
{
  __asm__ __volatile__("mv tp, %0" : : "r" (x));
}

static inline void 
set_cpuid(uint32_t x)
{
  __asm__ __volatile__("mv tp, %0" : : "r" (x));
}



/*
// xv6 riscv-5 magic
// Machine-mode Interrupt Enable
static inline uint64_t
r_menvcfg()
{
  uint64_t x;
  // __asm__ __volatile__("csrr %0, menvcfg" : "=r" (x) );
  __asm__ __volatile__("csrr %0, 0x30a" : "=r" (x) );
  return x;
}

static inline void 
w_menvcfg(uint64_t x)
{
  // __asm__ __volatile__("csrw menvcfg, %0" : : "r" (x));
  __asm__ __volatile__("csrw 0x30a, %0" : : "r" (x));
}

// Machine-mode Counter-Enable
static inline void 
w_mcounteren(uint64_t x)
{
  __asm__ __volatile__("csrw mcounteren, %0" : : "r" (x));
}

static inline uint64_t
r_mcounteren()
{
  uint64_t x;
  __asm__ __volatile__("csrr %0, mcounteren" : "=r" (x) );
  return x;
}


// Supervisor Timer Comparison Register
static inline uint64_t
r_stimecmp()
{
  uint64_t x;
  // __asm__ __volatile__("csrr %0, stimecmp" : "=r" (x) );
  __asm__ __volatile__("csrr %0, 0x14d" : "=r" (x) );
  return x;
}

*/

#endif /* !*/
