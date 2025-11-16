#include "inc/types.h"

#ifndef COMMON_FUNCS
#define COMMON_FUNCS

void *memset(void *buf, char c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, unsigned int n);


// Printf does not define where to put char, just formatting.
void printf(const char *fmt, ...);
void log(const char *fmt);

#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)

#endif /* !SIMPLE UTIL FUNCTIONS*/