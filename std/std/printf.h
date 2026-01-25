#ifndef PRINTING_H
#define PRINTING_H

#include "types.h"
#include "defs.h"

void printf(const char *fmt, ...);

int snprintf(char *buf, size_t size, const char *fmt, ...);

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

#endif