#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#include "defs.h"
#include "constants.h"
#include "types.h"

int32_t snprintf(uint8_t *buf, size_t size, const uint8_t *fmt, ...);

int32_t vsnprintf(uint8_t *buf, size_t size, const uint8_t *fmt, va_list args);

void printf(const uint8_t *fmt, ...);

#endif /* _LIBC_STDIO_H */
