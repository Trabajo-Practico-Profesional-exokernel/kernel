#include "filesystem.h"
#include "block.h"
#include "std/printf.h"
#include "std/string.h"

superblock_t super;
raw_block_t i_bmap;
raw_block_t d_bmap;

void fs_init(void){
    Block block;
    block_read(0, (char *) &block); 

    if(block.super_block.magic_number == MAGIC_NUMBER){
        super = block.super_block; 
        printf("Superblock formatted!\n");
    }else{
        fs_mkfs(); 
    }
}

#define T_DIR  2
#define T_FILE 1

void inode_load(uint32_t inum, inode_t *out_inode) {
    int block_idx = FIRST_INODE_POSITION + (inum / INODES_PER_BLOCK);
    int offset = inum % INODES_PER_BLOCK;
    
    Block block;
    block_read(block_idx, (char *)&block);
    *out_inode = block.inodes[offset];
}

int fs_mkdir(char *filepath){
    printf("new path name: %s", filepath);

    


}

void fs_tree_recursive(uint32_t inum, int level) {
    inode_t inode;
    inode_load(inum, &inode);

    if (inode.type != T_DIR) {
        return;
    }

    for (int i = 0; i < DIRECT_POINTERS; i++) {
        int phys_block = inode.direct[i];
        
        if (phys_block == 0) continue;

        Block block;
        block_read(phys_block, (char *)&block);

        int entries_count = BLOCK_SIZE / sizeof(dirent_t);

        for (int j = 0; j < entries_count; j++) {
            if (strlen(block.dirents[j].name) == 0) continue;

            char *name = block.dirents[j].name;
            uint32_t child_inum = block.dirents[j].inode;

            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }

            for (int k = 0; k < level; k++) {
                printf("    "); 
            }
            
            printf("|-- %s\n", name);

            inode_t child_inode;
            inode_load(child_inum, &child_inode);
            
            if (child_inode.type == T_DIR) {
                fs_tree_recursive(child_inum, level + 1);
            }
        }
    }
}

void print_filesystem(){
    printf("PRINTING FILESYSTEM (Tree View):\n");
    printf("/ (Root)\n"); 
    
    fs_tree_recursive(0, 0);
    
    printf("\n");
}

int fs_mkfs(void) {
    printf("formatting superblock!!\n");
    
    for(int i = 0; i < FILESYSTEM_TOTAL_BLOCKS; i++){
        set_zero_block(i);
    }

    Block block;

    super.magic_number = MAGIC_NUMBER;
    super.size_disk = FILESYSTEM_TOTAL_BLOCKS * BLOCK_SIZE;
    super.block_size = BLOCK_SIZE;
    super.num_inodes = INODES_PER_BLOCK * INODE_BLOCKS; 
    super.num_data_blocks = DATA_REGION_BLOCKS;         
    super.num_blocks_inodes = INODE_BLOCKS;
    
    block.super_block = super;
    block_write(SUPER_BLOCK_POSITION, (char *)&block);

    memset(i_bmap.data, 0, BLOCK_SIZE);
    memset(d_bmap.data, 0, BLOCK_SIZE);

    i_bmap.data[0] |= 1; 

    d_bmap.data[0] |= 1;

    block.raw = i_bmap;
    block_write(IMAP_POSITION, (char *)&block);

    block.raw = d_bmap;
    block_write(DMAP_POSITION, (char *)&block);

    block_read(FIRST_INODE_POSITION, (char *)&block);

    block.inodes[0].type = T_DIR;
    block.inodes[0].size = 2 * sizeof(dirent_t); 
    block.inodes[0].link_counter = 1;
    block.inodes[0].direct[0] = FIRST_DATA_BLOCK_POSITION; 

    block_write(FIRST_INODE_POSITION, (char *)&block);

    memset(&block, 0, sizeof(Block));

    strcpy(block.dirents[0].name, ".");
    block.dirents[0].inode = 0;

    strcpy(block.dirents[1].name, "..");
    block.dirents[1].inode = 0;

    block_write(FIRST_DATA_BLOCK_POSITION, (char *)&block);

    printf("Disk formatted successfully with Root Directory!\n");
    print_filesystem();
    return 0;
}