#include "block.h"
#include "std/string.h"

int block_read(int block, char *mem) {
    int res = disk_read(block, mem, BLOCK_SIZE);
    return res;
}

int block_write(int block, char *mem) {
    int res = disk_write(mem, block, BLOCK_SIZE);
    return res;
}

int set_zero_block(int block_index) {

    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    int res = disk_write(buffer, block_index, BLOCK_SIZE);
    
    return res;
}
