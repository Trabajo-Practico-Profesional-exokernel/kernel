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
#define APP_IND_COORDINATOR 4
#define APP_IND_EXAMPLE_DAPP 5
#define APP_IND_FILESYSTEM 6
#define APP_IND_FS/CAT 7
#define APP_IND_FS/CHMOD 8
#define APP_IND_FS/CHOWN 9
#define APP_IND_FS/LINK 10
#define APP_IND_FS/LS 11
#define APP_IND_FS/MKDIR 12
#define APP_IND_FS/RMDIR 13
#define APP_IND_FS/STAT 14
#define APP_IND_FS/TOUCH 15
#define APP_IND_FS/UNLINK 16
#define APP_IND_HELLO_WORLD 17
#define APP_IND_INFINITE_LOOP 18
#define APP_IND_KILL 19
#define APP_IND_MALLOC_PROGRAM 20
#define APP_IND_PAGE_FAULT 21
#define APP_IND_PERIODIC_YIELD 22
#define APP_IND_PIPE 23
#define APP_IND_PROC_A 24
#define APP_IND_PROC_B 25
#define APP_IND_READ_WRITE_SHELL 26
#define APP_IND_SHELL 27
#define APP_IND_SIMPLE_FRK 28
#define APP_COUNT 29

#endif
