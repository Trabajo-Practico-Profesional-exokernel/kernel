#ifndef _CONSOLE_DEBUG_H
#define _CONSOLE_DEBUG_H

#include "types.h"
#include "stdio.h"

enum DebugPrintMode {
    ENABLE,
    DISABLE  
};

extern enum DebugPrintMode actual_debug_print_mode;

void enable_debug_print(void);
void disable_debug_print(void);
void debug_printf(const char *fmt, ...);

#endif