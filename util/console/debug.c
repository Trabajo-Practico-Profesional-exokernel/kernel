#include "debug.h"
//#include "stdio.h"
#include "colors.h"
#include "types.h"
#include "defs.h"
#include "constants.h"
#include "stdio.h"

enum DebugPrintMode actual_debug_print_mode = ENABLE;

void enable_debug_print(void) {
    actual_debug_print_mode = ENABLE;
}

void disable_debug_print(void) {
    actual_debug_print_mode = DISABLE;
}

void debug_printf(const uint8_t *fmt, ...) {
    if (actual_debug_print_mode != ENABLE) return;

    uint8_t buf[256];
    va_list args;
    va_start(args, fmt);
    
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    printPurple((const uint8_t*)"[DEBUG] ");
    printf((const uint8_t*)"%s\n", buf); 
}