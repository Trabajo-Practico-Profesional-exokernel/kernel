#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H

#include "types.h"

void *memset(void *dst, int32_t c, size_t len);
void *memcpy(void *dst, const void *src, size_t len);
void *memmove(void *dst, const void *src, size_t len);
int32_t memcmp(const void *s1, const void *s2, size_t len);
void *memfind(const void *s, int32_t c, size_t len);

size_t strlen(const uint8_t *s);
size_t strnlen(const uint8_t *s, size_t size);

uint8_t *strcpy(uint8_t *dst, const uint8_t *src);
uint8_t *strncpy(uint8_t *dst, const uint8_t *src, size_t size);
uint8_t *strcat(uint8_t *dst, const uint8_t *src);
size_t strlcpy(uint8_t *dst, const uint8_t *src, size_t size);

int32_t strcmp(const uint8_t *s1, const uint8_t *s2);
int32_t strncmp(const uint8_t *s1, const uint8_t *s2, size_t size);

uint8_t *strchr(const uint8_t *s, int32_t c);
uint8_t *strtok(uint8_t *str, const uint8_t *delim);

#endif /* _LIBC_STRING_H */
