#ifndef PRINTING_H
#define PRINTING_H

#include "inc/types.h"

void printf(const char *fmt, ...);

void printGreen(const char* text);
  
void printRed(const char* text);
  
void printYellow(const char* text);

int snprintf(char *buf, size_t size, const char *fmt, ...);

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

#endif