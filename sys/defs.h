#ifndef BASIC_DEFS
#define BASIC_DEFS

// Compiler built-in wrappers for variable argument functions
typedef __builtin_va_list va_list;
#define va_start(v, l)  __builtin_va_start(v, l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v, l)    __builtin_va_arg(v, l)

// Utility Macros for limits, memory alignment, and structure access
#define MIN(a, b) \
    ({ typeof(a) _a = (a); typeof(b) _b = (b); _a <= _b ? _a : _b; })

#define MAX(a, b) \
    ({ typeof(a) _a = (a); typeof(b) _b = (b); _a >= _b ? _a : _b; })

#define ROUNDDOWN(a, n) \
    ({ uint32_t _a = (uint32_t) (a); (typeof(a)) (_a - _a % (n)); })

#define ROUNDUP(a, n) \
    ({ uint32_t _n = (uint32_t) (n); (typeof(a)) (ROUNDDOWN((uint32_t) (a) + _n - 1, _n)); })

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define offsetof(type, member)   __builtin_offsetof(type, member)
#define align_up(value, align)   (((value) + (align) - 1) & ~((align) - 1))
#define is_aligned(value, align) (((value) & ((align) - 1)) == 0)

#endif