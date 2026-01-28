/* blockFake.c */

#include "common.h"
#include "block.h"
#include "stdio.h"
#include "string.h"
#include "console/debug.h"


// Prototipos de las syscalls
extern int disk_read(unsigned int disk_sector, char* buffer, unsigned int read_len);
extern int disk_write(char* buffer, unsigned int disk_sector, unsigned int write_len);

void block_init(void) {
}

int block_read(int block, char *mem) {
    char local_buffer[BLOCK_SIZE];

    
    int res = disk_read(block, local_buffer, BLOCK_SIZE);


    if (res < 0) {
        debug_printf("[BLOCK READ ERROR] Sector: %d\n", block);
        return res;
    }
    memcpy(mem, local_buffer, BLOCK_SIZE);
    
    return res;
}

int block_write(int block, char *mem) {
    char local_buffer[BLOCK_SIZE];
    memcpy(local_buffer, mem, BLOCK_SIZE);
    int res = disk_write(local_buffer, block, BLOCK_SIZE);
    return res;
}

void bzero_block(char *block) {
    memset(block, 0, BLOCK_SIZE);
}
