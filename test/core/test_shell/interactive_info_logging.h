
#ifndef INTERACTIVE_INFO_LOGGING
#define INTERACTIVE_INFO_LOGGING

#include "types.h"

int enable_clock_yield_logging(char* args);
int disable_clock_yield_logging(void);

int enable_clock_yield_interactive(char* args);
int disable_clock_yield_interactive(void);

#endif

