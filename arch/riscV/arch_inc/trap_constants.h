#ifndef INC_TRAP_CONSTANTS
#define INC_TRAP_CONSTANTS
#include "inc/types.h"

#define MIE_STIE (1L << 5)  // supervisor timer
#define SIE_STIE (1L << 5)  // supervisor timer in S-Mode

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

// ** geppeto**
//#define write_csr_val(reg, val) ({ \
//    __asm__ __volatile__ ("csrw " #reg ", %0" :: "rK"(val)); })

#endif /* !*/
