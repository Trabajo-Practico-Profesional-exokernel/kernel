#include "arch/mem.h" //physical alloc.
#include "arch_inc/mem_constants.h" 
#include "arch/disk.h" // reading disk

#include "proc_disk_loading.h"
#include "console/debug.h"


#define APP_HEADERS_OFFSET 0
#define APP_HEADERS_MAGIC 0x41505053

struct BinaryAppEntry* _binary_user_apps = NULL;
uint32_t _binary_app_count = 0;

struct AppsHeadersDiskInfo {
    uint32_t magic_num;
    uint32_t app_count;
};


void init_proc_headers(void){
    // Read first 8 bytes! to check magic num and app count
    int curr_offset = APP_HEADERS_OFFSET;
    struct AppsHeadersDiskInfo headers_info;

    int remaining = read_disk((void *) &headers_info, curr_offset, 8);
    if(remaining > 0){
        PANIC("Failed to read app headers from disk remaining %d not read!", remaining);
    }
    curr_offset+=8;

    // Numbers are little endian

    if(headers_info.magic_num != APP_HEADERS_MAGIC){
        PANIC("Failed to read app headers from disk, invalid magic num %x!", headers_info.magic_num);
    }
    int headers_size = sizeof(struct BinaryAppEntry) * headers_info.app_count;

    int pages = align_up(headers_size, PAGE_SIZE) / PAGE_SIZE;

    printf("App headers info: magic num %x, app count %d, total headers size %d, pages to alloc %d\n", 
        headers_info.magic_num, headers_info.app_count, headers_size, pages);
        
    _binary_user_apps = (struct BinaryAppEntry*) alloc_pages(pages);

    remaining = read_disk((void *) _binary_user_apps, curr_offset, headers_size);

    if(remaining > 0){
        PANIC("Failed to read app headers from disk remaining %d not read!", remaining);
    }

    set_user_fs_start(curr_offset + headers_size);

    _binary_app_count = headers_info.app_count;
    printf("App headers loaded to memory at %x\n", _binary_user_apps);
    for(uint32_t i=0; i<_binary_app_count; i++){
        printf("App %u: name %s, offset %u, size %u\n", i, _binary_user_apps[i].name, 
            _binary_user_apps[i].start, _binary_user_apps[i].size);
    }

}

int get_app_count(void);
struct BinaryAppEntry* get_app_from_ind(int ind);
struct BinaryAppEntry* get_app_from_name(char* name);

int load_app_code_to_user_mem(struct BinaryAppEntry* appInfo, uint32_t* pde_table);