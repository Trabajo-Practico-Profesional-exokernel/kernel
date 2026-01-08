#ifndef TESTS_KERNEL_FUNCTIONS
#define TESTS_KERNEL_FUNCTIONS

#include "test_common.h"

void test_paging_mechanisms(CTest *ctx);
void test_process_management(CTest *ctx);
void test_ipc_logic(CTest *ctx);
void test_page_permission_bits(CTest *ctx);
void test_process_lifecycle_simulation(CTest *ctx);
#endif