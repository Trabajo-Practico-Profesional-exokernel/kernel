#include "testing.h"
#include "std/printf.h"
#include "utils.h"
#include "test_common.h"

void run_early_boot_tests(void) {

    CTest suite = init_ctx();

    int suma = 2 + 2;
    CTEST_ASSERT_EQ(&suite, 4, suma, "Matematica basica");

    test_run(&suite);

    if (suite.failed_count > 0){
        
        for(;;){};
    }
        
}

int main_tests(){
    run_early_boot_tests();
    return 0;
}