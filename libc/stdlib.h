#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H

#include "types.h"

int32_t strtol(const uint8_t *s, uint8_t **endptr, int32_t base);

int32_t atoi(const uint8_t *s);

void itoa(uint32_t n, uint8_t *s);

void itohex(uint32_t n, uint8_t *s);

void reverse(uint8_t *s);

void int_to_string(int32_t n, uint8_t *s);

#endif /* _LIBC_STDLIB_H */
