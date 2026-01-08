#ifndef PRINTING_H
#define PRINTING_H

#include "inc/types.h"

enum DebugPrintMode {
    ON,
    OFF  
};

extern enum DebugPrintMode actual_debug_print_mode;

void debug_printf(const char *fmt, ...);

void printf(const char *fmt, ...);

void printGreen(const char* text);
  
void printRed(const char* text);
  
void printYellow(const char* text);

int snprintf(char *buf, size_t size, const char *fmt, ...);

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

void enable_debug_print(void);

void disable_debug_print(void);

#endif