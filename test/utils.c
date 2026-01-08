#include "test_common.h"
#include "std/printf.h"
#include "arch/mem.h"
#include "std/string.h"

int record_result(CTest* ctx, int condition, const char* desc, 
                          const char* file, int line, const char* error_msg, int val1, int val2) {
    if (ctx->count >= MAX_TESTS) {
        printf("[CTEST] ERROR: test limit exceded\n");
        return 0;
    }

    Test* actual_test = &ctx->tests[ctx->count++];
    strcpy(&actual_test->description, desc);
    strcpy(&actual_test->file, file);
    actual_test->line = line;

    if (condition) {
        actual_test->status = PASSED;
        ctx->passed_count++;
        strcpy(&actual_test->error_message, "/0");
    } else {
        actual_test->status = FAILED;
        ctx->failed_count++;
        snprintf(actual_test->error_message, TEST_MSG_SIZE, error_msg, val1, val2);
    }
    return 1;
}

CTest init_ctx(const char* test_name){
    CTest ctx;
    memset(&ctx, 0, sizeof(CTest));
    strcpy(ctx.test_name, test_name);
    ctx.elements = 0;
    ctx.count = 0;
    ctx.passed_count = 0;
    ctx.failed_count = 0;
    return ctx;
}

int test_run(CTest* ctx) {
    printf("\n=== INIT %s TESTS ===\n", ctx->test_name);

    for (int i = 0; i < ctx->count; i++) {
        Test* t = &ctx->tests[i];

        printf("[TEST %x] %s ",i, t->description);

        if (t->status == PASSED) {
            printGreen(" PASSED \n");
        } else {
            printRed(" FAILED ");
            printf(" (%s:%d) -> %s\n", t->file, t->line, t->error_message); 
        }
    }

    printf("----------------------------------------\n");
    printf("Total: %d | ", ctx->count);
    printGreen("Passed: "); printf("%d | ", ctx->passed_count);
    printRed("Failed: "); printf("%d\n", ctx->failed_count);

    int success = ctx->failed_count == 0;

    if (success) {
        printGreen("\n>>> TESTS SUCCESSED <<<\n");
    } else {
        printRed("\n>>> TESTS FAILED <<<\n");
    }
    printf("\n=== FINISH %s TESTS ===\n\n\n", ctx->test_name);
    
    return success;
}