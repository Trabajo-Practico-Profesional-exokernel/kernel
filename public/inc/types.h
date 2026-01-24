#ifndef BASIC_TYPES
#define BASIC_TYPES
/*
* FROM xv6 inc/types.h with some extras from 1000 lines OS 
*/

#ifndef NULL
#define NULL ((void *) 0)
#endif

// Represents true-or-false values
typedef _Bool bool;
enum { false, true };

// Explicitly-sized versions of integer types
typedef __signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;


// Pointers and addresses are 32 bits long.
// We use pointer types to represent virtual addresses,
// uintptr_t to represent the numerical values of virtual addresses,
// and physaddr_t to represent physical addresses.

typedef uint32_t uintptr_t;
typedef int32_t intptr_t;


// size_t is used for memory object sizes.
typedef uintptr_t size_t;

// paddr_t physical address number
typedef uintptr_t paddr_t;

// vaddr_t virtual address number
typedef uintptr_t vaddr_t;

// Page numbers are 32 bits long.? maybe depends on 32 bits vs 64 bits too.
typedef uint32_t ppn_t;



// ssize_t is a signed version of ssize_t, used in case there might be an
// error return.
typedef intptr_t ssize_t;

// off_t is used for file offsets and lengths.
typedef intptr_t off_t;

// Efficient min and max operations
#define MIN(_a, _b)                                                            \
	({                                                                     \
		typeof(_a) __a = (_a);                                         \
		typeof(_b) __b = (_b);                                         \
		__a <= __b ? __a : __b;                                        \
	})
#define MAX(_a, _b)                                                            \
	({                                                                     \
		typeof(_a) __a = (_a);                                         \
		typeof(_b) __b = (_b);                                         \
		__a >= __b ? __a : __b;                                        \
	})
/*
// Rounding operations (efficient when n is a power of 2)
// Round down to the nearest multiple of n
#define ROUNDDOWN(a, n)                                                        \
	({                                                                     \
		uint32_t __a = (uint32_t) (a);                                 \
		(typeof(a)) (__a - __a % (n));                                 \
	})
// Round up to the nearest multiple of n
#define ROUNDUP(a, n)                                                          \
	({                                                                     \
		uint32_t __n = (uint32_t) (n);                                 \
		(typeof(a)) (ROUNDDOWN((uint32_t) (a) + __n - 1, __n));        \
	})
*/
#define ROUNDDOWN(a, n) ((a) & ~((n) - 1))
#define ROUNDUP(a, n)   (((a) + (n) - 1) & ~((n) - 1))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))



// Which should be? xv6 vs 1000 os one
// Return the offset of 'member' relative to the beginning of a struct type
//#define offsetof(type, member) ((size_t) (&((type *) 0)->member))

#define offsetof(type, member)   __builtin_offsetof(type, member)

#define align_up(value, align)   ROUNDUP(value, align)
#define is_aligned(value, align) (((value) & ((align) - 1)) == 0)

#define va_list  __builtin_va_list
#define va_start __builtin_va_start
#define va_end   __builtin_va_end
#define va_arg   __builtin_va_arg





#endif /* !BASIC_TYPES */
