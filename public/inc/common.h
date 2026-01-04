
#ifndef COMMON_FUNCS
#define COMMON_FUNCS

#include "inc/types.h"
#include "std/printf.h"
// #include "std/string.h"

#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)

#define UNUSED_ARGUMENT(x) (void) x;

#endif /* !SIMPLE UTIL FUNCTIONS*/