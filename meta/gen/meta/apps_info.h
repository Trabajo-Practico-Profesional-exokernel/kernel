#ifndef META_APPS_INFO
#define META_APPS_INFO


#include "inc/types.h"

struct AppBinaryInfo {
    void *start;
    size_t size;
};



#define APP_IND_SHELL 0
#define APP_IND_TESTS_SHELL 1
#define APP_IND_FILESYSTEM 2
#define APP_IND_HELLO_WORLD 3
#define APP_IND_PERIODIC_YIELD 4
#define APP_IND_PROC_A 5
#define APP_IND_PROC_B 6
#define APP_IND_RM 7
#define APP_IND_STAT 8
#define APP_IND_TOUCH 9
#define APP_COUNT 10

#endif
