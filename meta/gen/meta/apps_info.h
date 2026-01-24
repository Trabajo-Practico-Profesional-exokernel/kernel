#ifndef META_APPS_INFO
#define META_APPS_INFO


#include "inc/types.h"

struct AppBinaryInfo {
    void *start;
    size_t size;
};



#define APP_IND_TOUCH 0
#define APP_IND_RM 1
#define APP_IND_STAT 2
#define APP_IND_CAT 3
#define APP_IND_FILESYSTEM 4
#define APP_IND_HELLO_WORLD 5
#define APP_IND_KALLOC_PROGRAM 6
#define APP_IND_PAGE_FAULT 7
#define APP_IND_PERIODIC_YIELD 8
#define APP_IND_PROC_A 9
#define APP_IND_PROC_B 10
#define APP_IND_READ_WRITE_SHELL 11
#define APP_IND_SHELL 12
#define APP_IND_TESTS_SHELL 13
#define APP_COUNT 14

#endif
