#ifndef UTILS_FUNCTIONS
#define UTILS_FUNCTIONS

#include "test_common.h"

int record_result(CTest* ctx, int condition, const char* desc, 
    const char* file, int line, const char* error_msg, int val1, int val2);

int test_run(CTest* ctx);

CTest init_ctx(const char* test_name);
#endif