#ifndef PROC_DISK_LOADING_H
#define PROC_DISK_LOADING_H

#include "types.h"


// Should be the same as the .py that builds the image of apps!
#define APP_ENTRY_SIZE 40 
#define NAME_MAX_LEN APP_ENTRY_SIZE - 8 

struct  __attribute__((packed)) BinaryAppEntry {
    char name[NAME_MAX_LEN];
    uint32_t start;
    uint32_t size;
};

_Static_assert(sizeof(struct BinaryAppEntry) == APP_ENTRY_SIZE, "Bad binary app entry struct size!");



int get_app_count(void);
struct BinaryAppEntry* get_app_from_ind(int ind);
int get_app_from_name(char* name);

int load_app_code_to_user_mem(const struct BinaryAppEntry* app_info, 
            vaddr_t* curr_vaddr, uint32_t* pde_table);
#endif