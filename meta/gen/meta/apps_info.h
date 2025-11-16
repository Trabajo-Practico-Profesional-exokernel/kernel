#ifndef META_APPS_INFO
#define META_APPS_INFO


#include "inc/types.h"

struct AppBinaryInfo {
    void *start;
    size_t size;
};



#define APP_IND_SHELL 0
#define APP_IND_HELLO_WORLD 1
#define APP_IND_PROC_A 2
#define APP_IND_PROC_B 3

#endif
