#include "string.h"

extern void * malloc(size_t size);

char *strdup(const char *src)
{
    if (!src)
        return NULL;

    size_t len = strlen(src) + 1;   // +1 for '\0'

    char *dup = malloc(len);
    if (!dup)
        return NULL;

    memcpy(dup, src, len);

    return dup;
}