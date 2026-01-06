#ifndef TEST_COMMON_FUNCTIONS
#define TEST_COMMON_FUNCTIONS

#define MAX_TESTS 50
#define TEST_MSG_SIZE 128
#define MAX_FILE_NAME_SIZE 32

// Test status
typedef enum {
    IDLE,       // 0 status code
    RUNNING,    // 1 status code
    PASSED,     // 2 status code
    FAILED,      // 3 status code
    SKIPPED
} Status;


// Test object structure
typedef struct {
    int id;
    char description[TEST_MSG_SIZE];      // User provided description
    Status status;      
    char error_message[TEST_MSG_SIZE];    // Message to display on error
    char file[MAX_FILE_NAME_SIZE];
    int line;

} Test;

// Main Test
typedef struct {
    Test tests[MAX_TESTS];         // Limit of 50 tests per parent, might change later
    int elements;           // Counter of elements
    int count;
    int passed_count;
    int failed_count;
} CTest;

#define CTEST_ASSERT_EQ(ctx, expected, actual, desc) \
    record_result(ctx, (expected) == (actual), desc, __FILE__, __LINE__, "Expected: %x, Got: %x", (int)(expected), (int)(actual))

#define CTEST_ASSERT_TRUE(ctx, condition, desc) \
    record_result(ctx, (condition) == true, desc, __FILE__, __LINE__, "Expected TRUE, got FALSE", 0, 0)

#define CTEST_ASSERT_FALSE(ctx, condition, desc) \
    record_result(ctx, (condition) == false, desc, __FILE__, __LINE__, "Expected FALSE, got TRUE", 0, 0)

#define CTEST_ASSERT_NOT_NULL(ctx, ptr, desc) \
    record_result(ctx, (ptr) != NULL, desc, __FILE__, __LINE__, "NULL detected", 0, 0)

#endif