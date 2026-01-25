#ifndef BASIC_TYPES
#define BASIC_TYPES

// Base definition for null values
#define NULL ((void *) 0)

// Base definition for boolean values
typedef _Bool bool;
enum { false, true };

// Explicit bit-sized integers with defined signedness, architecture independent
typedef __signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

// Standard types for generic memory address manipulation and object sizes
typedef uint32_t uintptr_t; // Numeric value of a pointer
typedef int32_t intptr_t;   // Signed version for pointer arithmetic
typedef uintptr_t size_t;   // Memory object size
typedef intptr_t ssize_t;   // Signed size, useful for error returns
typedef intptr_t off_t;     // File offsets and lengths

// Kernel abstractions to distinguish physical/virtual addresses and pages
typedef uintptr_t paddr_t;  // Physical address
typedef uintptr_t vaddr_t;  // Virtual address
typedef uint32_t ppn_t;     // Physical Page Number

#endif /* BASIC_TYPES */