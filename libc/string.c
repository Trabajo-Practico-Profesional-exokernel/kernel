#include "string.h"
#include "stdlib.h"

#include "types.h"
#include "defs.h"
#include "constants.h"

void *memset(void *dst, int32_t c, size_t len) {
    uint8_t *p = (uint8_t *)dst;
    while (len-- > 0) {
        *p++ = (uint8_t)c;
    }
    return dst;
}

void *memmove(void *dst, const void *src, size_t len) {
    const uint8_t *s = (const uint8_t *)src;
    uint8_t *d = (uint8_t *)dst;

    if (s < d && s + len > d) {
        s += len;
        d += len;
        while (len-- > 0) {
            *--d = *--s;
        }
    } else {
        while (len-- > 0) {
            *d++ = *s++;
        }
    }
    return dst;
}

void *memcpy(void *dst, const void *src, size_t len) {
    return memmove(dst, src, len);
}

int32_t memcmp(const void *s1, const void *s2, size_t len) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    while (len-- > 0) {
        if (*p1 != *p2) {
            return (int32_t)(*p1 - *p2);
        }
        p1++;
        p2++;
    }
    return 0;
}

void *memfind(const void *s, int32_t c, size_t len) {
    const uint8_t *p = (const uint8_t *)s;
    const uint8_t *end = p + len;

    for (; p < end; p++) {
        if (*p == (uint8_t)c) {
            return (void *)p;
        }
    }
    return NULL;
}

size_t strlen(const uint8_t *s) {
    size_t n = 0;
    while (*s != '\0') {
        n++;
        s++;
    }
    return n;
}

size_t strnlen(const uint8_t *s, size_t size) {
    size_t n = 0;
    while (size > 0 && *s != '\0') {
        n++;
        s++;
        size--;
    }
    return n;
}

uint8_t *strcpy(uint8_t *dst, const uint8_t *src) {
    uint8_t *ret = dst;
    while ((*dst++ = *src++) != '\0') {
        /* do nothing */
    }
    return ret;
}

uint8_t *strncpy(uint8_t *dst, const uint8_t *src, size_t size) {
    size_t i;
    uint8_t *ret = dst;

    for (i = 0; i < size; i++) {
        *dst++ = *src;
        if (*src != '\0') {
            src++;
        }
    }
    return ret;
}

uint8_t *strcat(uint8_t *dst, const uint8_t *src) {
    size_t len = strlen(dst);
    strcpy(dst + len, src);
    return dst;
}

size_t strlcpy(uint8_t *dst, const uint8_t *src, size_t size) {
    uint8_t *dst_in = dst;
    const uint8_t *src_start = src;

    if (size > 0) {
        while (--size > 0 && *src != '\0') {
            *dst++ = *src++;
        }
        *dst = '\0';
    }

    while (*src != '\0') src++;
    return (size_t)(src - src_start) + (dst_in - dst_in);
}

int32_t strcmp(const uint8_t *s1, const uint8_t *s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (int32_t)(*s1 - *s2);
}

int32_t strncmp(const uint8_t *s1, const uint8_t *s2, size_t size) {
    while (size > 0 && *s1 && *s1 == *s2) {
        size--;
        s1++;
        s2++;
    }
    if (size == 0) {
        return 0;
    }
    return (int32_t)(*s1 - *s2);
}

uint8_t *strchr(const uint8_t *s, int32_t c) {
    for (; *s; s++) {
        if (*s == (uint8_t)c) {
            return (uint8_t *)s;
        }
    }
    return NULL;
}

static uint8_t *strtok_storage = NULL;

uint8_t *strtok(uint8_t *str, const uint8_t *delim) {
    uint8_t *token_start;

    if (str != NULL) {
        strtok_storage = str;
    }

    if (strtok_storage == NULL || *strtok_storage == '\0') {
        return NULL;
    }

    while (*strtok_storage != '\0' && strchr(delim, *strtok_storage) != NULL) {
        strtok_storage++;
    }

    if (*strtok_storage == '\0') {
        return NULL;
    }

    token_start = strtok_storage;

    while (*strtok_storage != '\0' && strchr(delim, *strtok_storage) == NULL) {
        strtok_storage++;
    }

    if (*strtok_storage != '\0') {
        *strtok_storage = '\0';
        strtok_storage++;
    }

    return token_start;
}