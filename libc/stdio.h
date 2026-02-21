#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#include "defs.h"
#include "constants.h"
#include "types.h"

int32_t snprintf(uint8_t *buf, size_t size, const uint8_t *fmt, ...);

int32_t vsnprintf(uint8_t *buf, size_t size, const uint8_t *fmt, va_list args);

void printf(const char *fmt, ...);


#ifdef IS_VERBOSE
#define VERBOSE_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define VERBOSE_PRINTF(fmt, ...) do {} while(0)
#endif


#ifdef IS_TESTING
#define TESTING_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define TESTING_PRINTF(fmt, ...) do {} while(0)
#endif



#endif /* _LIBC_STDIO_H */
