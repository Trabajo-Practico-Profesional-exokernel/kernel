#ifndef META_APPS_INFO
#define META_APPS_INFO


#include "types.h"

struct AppBinaryInfo {
    void *start;
    size_t size;
};



#define APP_IND_TOUCH 0
#define APP_IND_RM 1
#define APP_IND_STAT 2
#define APP_IND_CAT 3
#define APP_IND_CONSOLE 4
#define APP_IND_COORDINATOR 5
#define APP_IND_FILESYSTEM 6
#define APP_IND_HELLO_WORLD 7
#define APP_IND_INFINITE_LOOP 8
#define APP_IND_KALLOC_PROGRAM 9
#define APP_IND_KILL 10
#define APP_IND_PAGE_FAULT 11
#define APP_IND_PERIODIC_YIELD 12
#define APP_IND_PIPE 13
#define APP_IND_PROC_A 14
#define APP_IND_PROC_B 15
#define APP_IND_READ_WRITE_SHELL 16
#define APP_IND_SHELL 17
#define APP_IND_TESTS_SHELL 18
#define APP_COUNT 19

#endif
