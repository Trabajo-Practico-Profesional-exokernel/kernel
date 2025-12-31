#include "filesystem.h"
#include "block.h"
#include "std/printf.h"
#include "std/string.h"



// Retorna el índice del inodo libre o -1 si no hay espacio
int fs_get_free_inode(void) {
    Block block_buf;
    
    for (int i = 0; i < INODE_BLOCKS; i++) {
        int current_block = FIRST_INODE_POSITION + i;
        
        block_read(current_block, (char *)&block_buf);

        for (int j = 0; j < INODES_PER_BLOCK; j++) {
            if (block_buf.inodes[j].type == 0) {
                return (i * INODES_PER_BLOCK) + j;
            }
        }
    }
    
    printf("[FS] Error: No free inodes available.\n");
    return -1;
}



void fs_release_inode(int inode_idx) {
    
    if (inode_idx < 0 || inode_idx >= (int)super.num_inodes) {
        printf("[FS] Error: Invalid inode index %d to free.\n", inode_idx);
        return;
    }

    int block_idx = FIRST_INODE_POSITION + (inode_idx / INODES_PER_BLOCK);
    int inode_offset = inode_idx % INODES_PER_BLOCK;

    Block block_buf;
    block_read(block_idx, (char *)&block_buf);

    if (block_buf.inodes[inode_offset].type == 0){
        printf("[FS] Warning: Inode %d was already free.\n", inode_idx);
        return;
    }

    memset(&block_buf.inodes[inode_offset], 0, sizeof(inode_t));

    block_write(block_idx, (char *)&block_buf);
    
    printf("[FS] Inode %d freed successfully.\n", inode_idx);
}