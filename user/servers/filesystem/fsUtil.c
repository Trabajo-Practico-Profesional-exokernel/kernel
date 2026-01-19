/* fsUtil.c Corregido */

#include "fs.h"
#include "block.h"
#include "util.h"
#include "fsUtil.h"
#include "common.h"

extern superblock_t super;
extern bmap_t map;
extern dir_t current_dir;
extern FileDescriptor table[MAX_OPEN_FILES];

int check_file_permission(int uid, int gid, inode_t *file, int mode_requested) {

    if (uid == 0) return 1;

    int mode_bits = file->mode;

    if (uid == file->uid) {
        return (OWNER_PERMS(mode_bits) & mode_requested) ? 1 : 0;
    }

    if (gid == file->gid) {
        return (GROUP_PERMS(mode_bits) & mode_requested) ? 1 : 0;
    }

    return (OTHERS_PERMS(mode_bits) & mode_requested) ? 1 : 0;
}


/* Function to get and set block index number from inode. */
int get_indirect_iblock(uint32_t iblock, int height, int index){
    DataBlock block;
    // Limpieza defensiva
    bzero((char*)&block, sizeof(DataBlock));
    
    block_read(super.beg_data + iblock, (char *) &block);
    if(height == 1){
        return block.pointers[index];
    }
    int blocks_per_pointer = 1, i;
    for(i = 1; i < height; i++){
        blocks_per_pointer *= super.pointers_per_block;
    }
    for(i = 0; i < super.pointers_per_block; i++){
        if(block.pointers[i] == -1) return -1;
        if(index < blocks_per_pointer){
            return get_indirect_iblock(block.pointers[i], height-1, index);
        }
        index -= blocks_per_pointer;
    }
    return -1;
}

int get_iblock(inode_t file, int index){
    if(index >= max_blocks_of_file()) return -1;
    if(index < 0) return -1;

    if(index < super.direct_pointers){
        return file.direct[index];
    }
    index -= super.direct_pointers;
    if(index < super.pointers_per_block){
        if(file.indirect1 == -1) return -1;
        return get_indirect_iblock(file.indirect1, 1, index);
    }
    index -= super.pointers_per_block;
    if(index < super.pointers_per_block*super.pointers_per_block){
        if(file.indirect2 == -1) return -1;
        return get_indirect_iblock(file.indirect2, 2, index);
    }
    index -= super.pointers_per_block*super.pointers_per_block;
    if(file.indirect3 == -1) return -1;
    return get_indirect_iblock(file.indirect3, 3, index);
}

int set_indirect_iblock(uint32_t iblock, int height, int index, int new_inum){
    DataBlock block;
    bzero((char*)&block, sizeof(DataBlock));
    block_read(super.beg_data + iblock, (char *) &block);
    
    if(height == 1){
        if(index >= super.pointers_per_block) return -1;
        block.pointers[index] = new_inum;
        block_write(super.beg_data + iblock, (char *) &block);
        return 0;
    }

    int blocks_per_pointer = 1, i, ret;
    for(i = 1; i < height; i++){
        blocks_per_pointer *= super.pointers_per_block;
    }
    for(i = 0; i < super.pointers_per_block; i++){
        if(index < blocks_per_pointer){
            if(block.pointers[i] == -1){
                int new_iblock = get_single_available_iblock();
                if(new_iblock < 0) return -1;
                DataBlock aux_block;
                for(int j = 0; j < super.pointers_per_block; j++) 
                    aux_block.pointers[j] = -1;
                block_write(super.beg_data + new_iblock, (char *) &aux_block);
                block.pointers[i] = new_iblock;
                block_write(super.beg_data + iblock, (char *) &block);
            }
            ret = set_indirect_iblock(block.pointers[i], height-1, index, new_inum);
            if(is_pointers_block_empty(block.pointers[i])){
                free_iblock(block.pointers[i]);
                block.pointers[i] = -1;
            }
            if(ret == 0) return 0;
        }
        index -= blocks_per_pointer;
    }
    return -1;
}
    
int set_iblock(inode_t *file, int index, int new_inum){
    if(index >= max_blocks_of_file()) return -1;
    int ppb = super.pointers_per_block, ret;

    if(index < super.direct_pointers){
        file->direct[index] = new_inum;
        return 0;
    }
    index -= super.direct_pointers;
    if(index < ppb){
        if(file->indirect1 == -1){
            int iblock = get_single_available_iblock();
            if(iblock < 0) return -1;
            DataBlock block;
            for(int i = 0; i < ppb; i++) block.pointers[i] = -1;
            block_write(super.beg_data + iblock, (char *) &block);
            file->indirect1 = iblock;
        }
        ret = set_indirect_iblock(file->indirect1, 1, index, new_inum);
        if(is_pointers_block_empty(file->indirect1)){
            free_iblock(file->indirect1);
            file->indirect1 = -1;
        }
        return ret;
    }
    index -= ppb;
    if(index < ppb*ppb){
        if(file->indirect2 == -1){
            int iblock = get_single_available_iblock();
            if(iblock < 0) return -1;
            DataBlock block;
            for(int i = 0; i < ppb; i++) block.pointers[i] = -1;
            block_write(super.beg_data + iblock, (char *) &block);
            file->indirect2 = iblock;
        }
        ret = set_indirect_iblock(file->indirect2, 2, index, new_inum);
        if(is_pointers_block_empty(file->indirect2)){
            free_iblock(file->indirect2);
            file->indirect2 = -1;
        }
        return ret;
    }
    index -= ppb*ppb;
    if(file->indirect3 == -1){
        int iblock = get_single_available_iblock();
        if(iblock < 0) return -1;
        DataBlock block;
        for(int i = 0; i < ppb; i++) block.pointers[i] = -1;
        block_write(super.beg_data + iblock, (char *) &block);
        file->indirect3 = iblock;
    }
    ret = set_indirect_iblock(file->indirect3, 3, index, new_inum);
    if(is_pointers_block_empty(file->indirect3)){
        free_iblock(file->indirect3);
        file->indirect3 = -1;
    }
    return ret;
}

/* Functions to manipulate the map of bits. */
int32_t get_single_available_inode(){
    for(int i = 0; i < super.num_inodes; i++){
        if((map.imap[i/8]&(1<<(7-i%8))) == 0){
            map.imap[i/8] |= (1<<(7-i%8));
            save_map();
            return i;
        }
    }
    return -1;
}

int32_t get_single_available_iblock(){
    for(int i = 0; i < super.num_data_blocks; i++){
        if((map.dmap[i/8]&(1<<(7-i%8))) == 0){
            map.dmap[i/8] |= (1<<(7-i%8));
            save_map();
            return i;
        }
    }
    return -1;
}

void free_iblock(int32_t inum){
    map.dmap[inum/8] &= ~(1<<(7-inum%8));
    save_map();
}

void free_inode(int32_t inum){
    map.imap[inum/8] &= ~(1<<(7-inum%8));
    save_map();
}

void save_map(){
    Block aux;
    aux.map = map;
    block_write(MAP_BLOCK, (char *) &aux);
}

/* Operations over Directory Control Block (DCB) */
bool_t is_dir_block_empty(int iblock){
    DataBlock block;
    bzero((char*)&block, sizeof(DataBlock));
    block_read(super.beg_data + iblock, (char *) &block);
    return (block.dir.files_inum[0] == -1);
} 

void remove_file_from_dir(inode_t * dir_inode, int ptr_to_remove){
    int block_index, current_iblock, next_iblock;
    int i;
    block_index = ptr_to_remove/super.pointers_per_dcb; 
    ptr_to_remove %= super.pointers_per_dcb;
    next_iblock = get_iblock(*dir_inode, block_index);

    do{
        current_iblock = next_iblock; 
        DataBlock block, next_block;
        bzero((char*)&block, sizeof(DataBlock));
        bzero((char*)&next_block, sizeof(DataBlock));

        block_read(super.beg_data + current_iblock, (char *) &block);

        for(i = ptr_to_remove; i < super.pointers_per_dcb-1; i++){
            memcpy((char*)block.dir.files_name[i], (char*)block.dir.files_name[i+1], MAX_FILE_NAME);
            block.dir.files_inum[i] = block.dir.files_inum[i+1];
        }

        next_iblock = get_iblock(*dir_inode, ++block_index);
        if(next_iblock != -1){
            block_read(super.beg_data + next_iblock, (char *) &next_block);
            memcpy((char*)block.dir.files_name[i], (char*)next_block.dir.files_name[0], MAX_FILE_NAME);
            block.dir.files_inum[i] = next_block.dir.files_inum[0];
        }else{
            bzero((char*)block.dir.files_name[i], MAX_FILE_NAME);
            block.dir.files_inum[i] = -1;
        }
        block_write(super.beg_data + current_iblock, (char *) &block);
        ptr_to_remove = 0;
    }while(next_iblock != -1);

    if(is_dir_block_empty(current_iblock) == TRUE){
        set_iblock(dir_inode, --block_index, -1); 
        free_iblock(current_iblock); 
    }
}

int find_file_in_dir(inode_t file, char * fileName, int* relIndex){
    DataBlock block;
    int iblock, num_blocks;

    num_blocks = (file.size + super.pointers_per_dcb - 1) / super.pointers_per_dcb;
    for(int i = 0; i < num_blocks; i++){
        iblock = get_iblock(file, i);
        bzero((char*)&block, sizeof(DataBlock));
        block_read(super.beg_data + iblock, (char *) &block);

        for(int j = 0; j < super.pointers_per_dcb; j++){
            if(block.dir.files_inum[j] != -1) {
                 // Usamos strncmp para seguridad
                 if (strncmp((char*) fileName, (char*) block.dir.files_name[j], MAX_FILE_NAME) == 0) {
                    if(relIndex != NULL){
                        *relIndex = i * super.pointers_per_dcb + j;
                    }
                    return block.dir.files_inum[j];
                 }
            }
        }
    }
    return -1;
}

int insert_file_in_dir(inode_t * dir, char * fileName, int32_t inum){
    int num_blocks = (dir->size + super.pointers_per_dcb - 1) / super.pointers_per_dcb;
    int32_t last_block_inum = get_iblock(*dir, num_blocks-1);
    if(last_block_inum < 0) return -1;

    DataBlock block;
    bzero((char*)&block, sizeof(DataBlock));
    block_read(super.beg_data + last_block_inum, (char *) &block);

    // Intentar insertar en bloque existente
    for(int i = 0; i < super.pointers_per_dcb; i++){
        if(block.dir.files_inum[i] == -1){
            // Copia segura. Limpiamos primero el destino.
            bzero((char*)block.dir.files_name[i], MAX_FILE_NAME);
            // Copiamos max MAX_FILE_NAME - 1 chars para asegurar null termination
            strncpy((char*)block.dir.files_name[i], fileName, MAX_FILE_NAME - 1);
            block.dir.files_inum[i] = inum;
            block_write(super.beg_data + last_block_inum, (char *) &block);
            return 0;
        }
    }

    // Nuevo bloque requerido
    int new_iblock = get_single_available_iblock();
    if(new_iblock < 0) return -1;

    DataBlock new_block;
    bzero((char*)&new_block, sizeof(DataBlock)); // Limpieza total
    for(int i = 0; i < super.pointers_per_dcb; i++){
        new_block.dir.files_inum[i] = -1;
    }

    // Insertar entrada
    strncpy((char*)new_block.dir.files_name[0], fileName, MAX_FILE_NAME - 1);
    new_block.dir.files_inum[0] = inum;
    
    num_blocks++;
    if(set_iblock(dir, num_blocks-1, new_iblock) < 0){
        free_iblock(new_iblock);
        return -1;
    }
    block_write(super.beg_data + new_iblock, (char *) &new_block);
    save_map();
    save_inode(current_dir.files_inum[0], *dir);
    return 0;
}

dir_t create_directory(int inum){
    dir_t new_dir;
    for(int i = 0; i < super.pointers_per_dcb; i++){
        bzero((char *) new_dir.files_name[i], MAX_FILE_NAME);
        new_dir.files_inum[i] = -1;
    }
    // Set . and ..
    strcpy((char*)new_dir.files_name[0], ".");
    strcpy((char*)new_dir.files_name[1], "..");
    new_dir.files_inum[0] = inum;
    new_dir.files_inum[1] = current_dir.files_inum[0];
    return new_dir;
}

bool_t is_directory_empty(inode_t dir){
    if(dir.size > 2) return FALSE;
    DataBlock block;
    bzero((char*)&block, sizeof(DataBlock));
    block_read(super.beg_data + dir.direct[0], (char *) &block);
    return (block.dir.files_inum[2] == -1);
}

/* Operations over inodes */
void save_inode(int index, inode_t inode){
    int iblock = index / super.inodes_per_block;
    Block block;
    bzero((char*)&block, sizeof(Block));
    block_read(super.beg_inodes + iblock, (char *) &block);
    block.inodes[index%super.inodes_per_block] = inode;
    block_write(super.beg_inodes + iblock, (char *) &block);
}

inode_t get_inode_per_inum(int index){
    int iblock = index / super.inodes_per_block;
    Block block;
    bzero((char*)&block, sizeof(Block));
    block_read(super.beg_inodes + iblock, (char *) &block);
    return block.inodes[index%super.inodes_per_block];
}

/* Operation on Table of Open Files */
int get_single_available_fd(){
    for(int fd = 0; fd < MAX_OPEN_FILES; fd++){
        if(table[fd].fd == -1) return fd;
    }
    return -1;
}

/* Operations on Files */
void free_all_data_blocks_indirect(int iblock, int height){
    if(height >= 1){
        DataBlock block;
        bzero((char*)&block, sizeof(DataBlock));
        block_read(super.beg_data + iblock, (char *) &block);
        for(int i = 0; i < super.pointers_per_block; i++){
            if(block.pointers[i] == -1) break;
            if(height > 1)
                free_all_data_blocks_indirect(block.pointers[i], height-1);
            free_iblock(block.pointers[i]);
        }
    }
}

void free_all_data_blocks(inode_t inode){
    int num_blocks = (inode.size + super.block_size -1) / super.block_size;
    for(int i = 0; i < super.direct_pointers && i < num_blocks; i++){
        free_iblock(inode.direct[i]);
    }
    if(inode.indirect1 != -1){
        free_all_data_blocks_indirect(inode.indirect1, 1);
        free_iblock(inode.indirect1);
    }
    if(inode.indirect2 != -1){
        free_all_data_blocks_indirect(inode.indirect2, 2);
        free_iblock(inode.indirect2);
    }
    if(inode.indirect3 != -1){
        free_all_data_blocks_indirect(inode.indirect3, 3);
        free_iblock(inode.indirect3);
    }
}

bool_t is_pointers_block_empty(int iblock){
    DataBlock block;
    bzero((char*)&block, sizeof(DataBlock));
    block_read(super.beg_data+iblock, (char*) &block);
    return (block.pointers[0] == -1);
}

/* General Purpose */
int max_blocks_of_file(){
    int aux = super.pointers_per_block;
    return aux * aux * aux + aux * aux + aux + super.direct_pointers; 
}

int blocks_used(){
    int cnt = 0;
    for(int i = 0; i < super.num_data_blocks; i++){
        cnt += ((map.dmap[i/8] & (1<<(7-i%8))) > 0);
    } 
    return cnt;
}

int inodes_used(){
    int cnt = 0;
    for(int i = 0; i < super.num_inodes; i++){
        cnt += ((map.imap[i/8] & (1<<(7-i%8))) > 0);
    } 
    return cnt;
}

