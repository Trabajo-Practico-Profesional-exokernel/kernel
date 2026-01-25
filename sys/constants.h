#ifndef CONSTANTS_H
#define CONSTANTS_H

//#include "stdio.h"
#include "std/printf.h"

#define NULL    ((void *) 0)
#define TRUE    1
#define FALSE   0

#define SUCCESS      0
#define ERROR       -1

#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)

#define UNUSED_ARGUMENT(x) (void) x;

#endif /* CONSTANTS_H */