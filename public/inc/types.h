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



#endif /* !BASIC_TYPES */
