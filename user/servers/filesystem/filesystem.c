#include "filesystem.h"
#include "block.h"
#include "std/printf.h"

superblock_t super;

void fs_init(void){
    Block block;
    block_read(0, (char *) &block); 

    // check if disk is formatted
    if(block.sb.magic_number == MAGIC_NUMBER){
        super = block.sb; // set superblock
        printf("Superblock formatted!\n");
    }else{
        fs_mkfs(); // format disk
    }
}

int fs_mkfs(void){
    printf("formatting superblock!!\n");
    for(int i = 0; i < FILESYSTEM_SIZE; i++){
        set_zero_block(i);
    }
    super = (superblock_t) {.magic_number = MAGIC_NUMBER};
    Block block;
    block.sb = super;
    block_write(0, (char *) &block);
    return 0;
}