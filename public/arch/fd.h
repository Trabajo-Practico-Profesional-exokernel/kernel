#ifndef FD
#define FD

#define MAX_SIZE_PATH 256

typedef struct {
    int in_use;               // Is this FD active?
    char path[MAX_SIZE_PATH];           // Full file path 
    size_t position;          // Current read/write position 
    size_t file_size;         // Total file size 
    int flag;
    int is_directory;         // 1 if this is a directory descriptor 
} File;

#endif
