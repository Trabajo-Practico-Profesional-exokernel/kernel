#include "testing.h"
#include "std/printf.h"
#include "utils.h"
#include "test_common.h"
#include "tests_kernel.h"

#ifdef IS_RISC
#include "test_riscv/tests_hardware.h"
#else
#include "test_x86/tests_hardware.h"
#endif

int run_early_boot_tests(void) {
    CTest suite = init_ctx("SELF-TEST KIT");

    CTEST_ASSERT_EQ(&suite, 1, 1, "Assert EQUAL test");
    CTEST_ASSERT_TRUE(&suite, 1, "Assert TRUE test");
    CTEST_ASSERT_FALSE(&suite, 0, "Assert FALSE test");
    CTEST_ASSERT_NOT_NULL(&suite, (void*)0x1234, "Assert NOT NULL test");
    CTEST_ASSERT_GT(&suite, 2, 1, "Assert GREATER THAN test");
    CTEST_ASSERT_GE(&suite, 1, 1, "Assert GREATER EQUAL test");
    CTEST_ASSERT_GE(&suite, 2, 1, "Assert GREATER EQUAL test");
    CTEST_ASSERT_LT(&suite, 1, 2, "Assert LESS THAN test");
    CTEST_ASSERT_LE(&suite, 1, 1, "Assert LESS EQUAL test");
    CTEST_ASSERT_LE(&suite, 1, 2, "Assert LESS EQUAL test");

    return test_run(&suite);
}

int run_kernel_tests(void) {
    CTest suite = init_ctx("RING 0");

    test_process_management(&suite);
    test_ipc_logic(&suite);
    test_process_lifecycle_simulation(&suite);

    return test_run(&suite);
}

int run_user_tests(void) {
    CTest suite = init_ctx("RING 3");
    return test_run(&suite);
}

int main_tests(){

    int success_1 = run_early_boot_tests();
    int success_2 = run_hardware_tests();
    int success_3 = run_kernel_tests();
    int success_4 = run_user_tests();

    if (success_1 && success_2 && success_3){
        printGreen("=============================\n");
        printGreen("===== KIT TESTS SUCCESS =====\n");
        printGreen("=============================\n\n");
        return 0;
    } else {

        printRed("============================\n");
        printRed("===== KIT TESTS FAILED =====\n");
        printRed("============================\n\n");
        for(;;){}
    }
    
}