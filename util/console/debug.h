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

#ifdef IS_TESTING
#define TESTING_DEBUG_PRINTF(fmt, ...) debug_printf(fmt, ##__VA_ARGS__)
#else
#define TESTING_DEBUG_PRINTF(fmt, ...) do {} while(0)
#endif

#ifdef IS_VERBOSE
#define VERBOSE_DEBUG_PRINTF(fmt, ...) debug_printf(fmt, ##__VA_ARGS__)

#ifdef DEBUG_LEVEL

#define VERBOSE_DEBUG_PRINTF_LV(level, fmt, ...) \
    do { \
        if ((level) <= DEBUG_LEVEL) { \
            debug_printf(fmt, ##__VA_ARGS__); \
        } \
    } while(0)

#else
#define VERBOSE_DEBUG_PRINTF_LV(level, fmt, ...) do {} while(0)
#endif

#else
#define VERBOSE_DEBUG_PRINTF(fmt, ...) do {} while(0)
#define VERBOSE_DEBUG_PRINTF_LV(level, fmt, ...) do {} while(0)
#endif


#endif