#include "arch/mem.h" //physical alloc.
#include "arch_inc/mem_constants.h" 
#include "arch/disk.h" // reading disk

#include "proc_disk_loading.h"
#include "console/debug.h"
#include "stdlib.h"
#include "string.h"
#include "arch/arch_init.h"


#define APP_HEADERS_OFFSET 0
#define APP_HEADERS_MAGIC 0x41505053

struct BinaryAppEntry* _binary_user_apps = NULL;
uint32_t _binary_app_count = 0;

struct AppsHeadersDiskInfo {
    uint32_t magic_num;
    uint32_t app_count;
};

// char user_proc_load[PAGE_SIZE];


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

int get_app_count(void){
    return _binary_app_count;
}

struct BinaryAppEntry* get_app_from_ind(int ind){
    if(ind < 0 || ind >= (int)_binary_app_count){
        return NULL;
    }
    return &_binary_user_apps[ind];
}

int get_app_from_name(char* name){
    int len_act = strlen((const uint8_t*)name) + 1;// include 0 byte

    for(uint32_t i=0; i<_binary_app_count; i++){
        if(strncmp((const uint8_t*)_binary_user_apps[i].name, (const uint8_t*)name, len_act) == 0){
            return i;
        }
    }
    return -1;
}

int load_app_code_to_user_mem(const struct BinaryAppEntry* app_info, 
            vaddr_t* curr_vaddr, uint32_t* pde_table){

    uint32_t curr_offset = app_info->start;
    int remaining = app_info->size;
    // paddr_t curr_page_paddr;
    // if(try_alloc_user_page(&curr_page_paddr) < 0){
    //     PANIC("Failed alloc user page in load code for process!\n");
    //     // debug_printf("Failed alloc user page in load code for process!\n");
    //     return;
    // }
    

    while(remaining > 0) {
        paddr_t curr_page_paddr = alloc_pages(1); // Alloc pages throws PANIC ALREADY!

 
        
        if(curr_offset == app_info->start){
            printf("PADDR START OF PROCESS '%s' 0x%x\n", app_info->name, curr_page_paddr);
        }

        // Handle the case where the data to be copied is smaller than the page size.
        size_t copy_size = (PAGE_SIZE <= remaining) ? PAGE_SIZE : remaining;


        int not_written = read_disk((void *) curr_page_paddr, 
            curr_offset, copy_size);

        if(not_written > 0){
            printf("Failed to read app '%s' content from disk remaining %d not written!\n", app_info->name,not_written);
            return -1;
        }

        curr_offset+=copy_size;
        remaining-=copy_size;

        // Map the loaded code to the VADDR of the user programs
        map_page(pde_table, *curr_vaddr, curr_page_paddr,
                 USER_PERMISSIONS_ALL);
        
        *curr_vaddr += PAGE_SIZE;
    }    
    return 0;
}
