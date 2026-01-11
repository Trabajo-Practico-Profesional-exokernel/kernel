/* fs.c */
#include "util.h"
#include "common.h"
#include "block.h"
#include "fs.h"
#include "fsUtil.h"
#include "inc/filesystem.h"


// Variables Globales del Filesystem
superblock_t super;
bmap_t map;
dir_t current_dir[PROCS_MAX]; // Modificado a array por proceso
char current_path[PROCS_MAX][MAX_PATH_NAME];
FileDescriptor table[PROCS_MAX][MAX_OPEN_FILES];

/* ------------------------------------------------------------------------- LÓGICA INTERNA DEL FILESYSTEM */

void fs_sync_current_dir(int proc) {
    // El inodo del directorio actual está siempre en la posición 0 (.)
    int dir_inum = current_dir[proc].files_inum[0];
    // Leemos el inodo actualizado desde disco
    inode_t dir_inode = get_inode_per_inum(dir_inum);

    // Por ahora solo usás punteros directos
    int block_index = dir_inode.direct[0];

    if (block_index < 0) {
        debug_printf("[FS] ERROR: current dir has no data block\n");
        return;
    }

    Block block;
    block_read(super.beg_data + block_index, (char *)&block);

    // Copiamos el directorio actualizado del disco a RAM para el proceso específico
    current_dir[proc] = block.data_block.dir;
}

void shell_ls(int proc) {
    DataBlock block;
    int i, j;
    inode_t dir_inode = get_inode_per_inum(current_dir[proc].files_inum[0]);

    int num_blocks = (dir_inode.size + super.pointers_per_dcb - 1) / super.pointers_per_dcb;

    printf("Nombre          Tipo    Inum    Size\n");
    printf("------------------------------------\n");

    for (i = 0; i < num_blocks; i++) {
        int current_iblock = get_iblock(dir_inode, i);
        if (current_iblock < 0) {
            continue;
        }
        block_read(super.beg_data + current_iblock, (char *)&block);
        for (j = 0; j < super.pointers_per_dcb; j++) {
            if (block.dir.files_inum[j] == -1) {
                continue;
            }

            inode_t file_inode = get_inode_per_inum(block.dir.files_inum[j]);
            int show_size = file_inode.size;
            if (file_inode.type == DIRECTORY && show_size >= 2) {
                show_size -= 2;
            }

            printf("%-15s %s \t%d \t%d\n",
                   (char *)block.dir.files_name[j],
                   (file_inode.type == DIRECTORY ? "D" : "F"),
                   (int)block.dir.files_inum[j],
                   show_size);
        }
    }
}

void fs_init(void) {
    block_init();
    Block block;
    block_read(0, (char *)&block);

    // check if disk is formatted
    if (block.sb.magic_number == MAGIC_NUMBER) {
        super = block.sb; // set superblock

        // load map
        block_read(super.beg_map, (char *)&block);
        map = block.map; // set bits map

        block_read(super.beg_data, (char *)&block);

        // initialize current dir and path for ALL processes
        for (int j = 0; j < PROCS_MAX; j++) {
            current_dir[j] = block.data_block.dir; // set root for everyone
            bcopy((unsigned char *)"/", (unsigned char *)current_path[j], 2);
        }

        // initialize open-files table
        for (int j = 0; j < PROCS_MAX; j++) {
            for (int i = 0; i < MAX_OPEN_FILES; i++) {
                table[j][i].fd = -1;
                bzero(table[j][i].name, MAX_PATH_NAME);
            }
        }

    } else {
        fs_mkfs(); // format disk
    }
}

int fs_mkfs(void) {
    char null_block[BLOCK_SIZE];
    bzero(null_block, BLOCK_SIZE);

    for (int i = 0; i < FS_SIZE; i++) {
        block_write(i, null_block);
    }

    // define superblock
    super = (superblock_t){
        .magic_number = MAGIC_NUMBER,
        .size_disk = FS_SIZE,
        .block_size = BLOCK_SIZE,
        .num_inodes = INODES_PER_BLOCK * INODES_BLOCKS,
        .num_data_blocks = FS_SIZE - 2 - INODES_BLOCKS,
        .num_blocks_inodes = INODES_BLOCKS,
        .beg_inodes = 1,
        .beg_map = 1 + INODES_BLOCKS,
        .beg_data = 2 + INODES_BLOCKS,
        .pointers_per_block = BLOCK_SIZE / 4,
        .pointers_per_dcb = POINTERS_PER_DCB,
        .inodes_per_block = INODES_PER_BLOCK,
        .direct_pointers = DIRECT_POINTERS};

    // mark inodes and block data as free
    for (int i = 0; i < IMAP_BYTES; i++)
        map.imap[i] = 0;
    for (int i = 0; i < DMAP_BYTES; i++)
        map.dmap[i] = 0;

    // Usamos una variable temporal para crear el root, luego la copiamos a todos los procesos
    dir_t root_dir;
    bzero((char *)&root_dir, sizeof(dir_t));

    // create root dir
    for (int i = 0; i < super.pointers_per_dcb; i++)
        root_dir.files_inum[i] = -1;

    bcopy((unsigned char *)".", (unsigned char *)root_dir.files_name[0], 2);
    bcopy((unsigned char *)"..", (unsigned char *)root_dir.files_name[1], 3);
    root_dir.files_inum[0] = root_dir.files_inum[1] = 0;

    // Asignamos el root dir a todos los procesos
    for (int j = 0; j < PROCS_MAX; j++) {
        current_dir[j] = root_dir;
    }

    // create inode to root dir
    inode_t iroot = (inode_t){
        .type = DIRECTORY,
        .link_counter = 1,
        .size = 2,
        .indirect1 = -1,
        .indirect2 = -1,
        .indirect3 = -1};
    for (int i = 0; i < super.direct_pointers; i++)
        iroot.direct[i] = -1;
    iroot.direct[0] = 0;

    // set inum and iblock of root as used
    map.imap[0] |= (1 << 7);
    map.dmap[0] |= (1 << 7);

    // writing to disk
    Block block;
    block.sb = super;

    block_write(0, (char *)&block); // writing superblock

    block.inodes[0] = iroot;
    block_write(1, (char *)&block); // writing first inode

    save_map(); // writing bits map

    block.data_block.dir = root_dir;              // Guardamos lo que configuramos arriba
    block_write(super.beg_data, (char *)&block); // writing root directory

    // initialize open-files table
    for (int j = 0; j < PROCS_MAX; j++) {
        for (int i = 0; i < MAX_OPEN_FILES; i++) {
            table[j][i].fd = -1;
            bzero(table[j][i].name, MAX_PATH_NAME);
        }
    }

    // initialize current path for all processes
    for (int j = 0; j < PROCS_MAX; j++) {
        bcopy((unsigned char *)"/", (unsigned char *)current_path[j], 2);
    }

    return 0;
}

int fs_open(char *fileName, int flags, int proc) {
    Block block;
    int ret;
    inode_t inode_dir = get_inode_per_inum(current_dir[proc].files_inum[0]);
    int existFile = find_file_in_dir(inode_dir, fileName, NULL);

    // Busca un FD disponible en la tabla del proceso
    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (table[proc][i].fd == -1) {
            fd = i;
            break;
        }
    }

    if (fd < 0) {
        return -1;
    }

    // if file doesn't exist and flags is RDWR or WRONLY, we must create
    // the file
    if (existFile == -1) {
        if (flags == FS_O_RDONLY) {
            return -1;
        }

        // Allocate inode
        int inum = get_single_available_inode();
        if (inum < 0) {
            return -1;
        }

        // set inode entries
        inode_t new_ifile = (inode_t){
            .type = FILE_TYPE,
            .link_counter = 1,
            .size = 0,
            .indirect1 = -1,
            .indirect2 = -1,
            .indirect3 = -1};
        for (int i = 0; i < super.direct_pointers; i++)
            new_ifile.direct[i] = -1;

        // update parent
        ret = insert_file_in_dir(&inode_dir, fileName, inum);
        if (ret < 0) {
            free_inode(inum);
            return -1;
        }
        inode_dir.size++;

        block_read(super.beg_data + inode_dir.direct[0], (char *)&block);
        current_dir[proc] = block.data_block.dir; // Actualizamos current_dir del proceso

        // write blocks to disk
        save_inode(inum, new_ifile);                                // writing new inode
        save_inode(current_dir[proc].files_inum[0], inode_dir); // writing new inode

        block.map = map;
        block_write(super.beg_map, (char *)&block); // writing bits map

        existFile = inum;
    }

    inode_t current_inode = get_inode_per_inum(existFile);

    if (current_inode.type == DIRECTORY && flags != FS_O_RDONLY) {
        return -1;
    }

    // create a new FileDescriptor instance
    FileDescriptor file;
    file.fd = fd;
    bcopy((unsigned char *)fileName, (unsigned char *)file.name, strlen(fileName) + 1);
    file.inode = existFile;
    file.flag = flags;
    file.rw_ptr = 0;

    // insert into the table
    table[proc][fd] = file;

    return fd;
}

int fs_close(int fd, int proc) {
    if (fd >= MAX_OPEN_FILES || table[proc][fd].fd == -1) {
        return -1;
    }

    inode_t current_inode = get_inode_per_inum(table[proc][fd].inode);

    // Erase file whether it's its last link
    if (current_inode.link_counter == 0) {

        // check if there isn't any other fd open for this inode within the process table
        // (Nota: Idealmente esto debería verificar todos los procesos si fuera una tabla global real,
        //  pero mantenemos el scope del proceso según la estructura solicitada)
        int found = 0;
        for (found = 0; found < MAX_OPEN_FILES; found++) {
            if (found != fd && table[proc][found].fd != -1 &&
                table[proc][found].inode == table[proc][fd].inode) {
                break;
            }
        }

        if (found == MAX_OPEN_FILES) {
            // free all data blocks associate with this file
            free_all_data_blocks(current_inode);
            // free its inode
            free_inode(table[proc][fd].inode);
            // save changes
            save_map();
        }
    }

    // close its fd
    table[proc][fd].fd = -1;

    return 0;
}

int fs_read(int fd, char *buf, int count, int proc) {
    int rw, index_block, iblock;
    inode_t current_inode;
    DataBlock block;
    if (count == 0) {
        return 0;
    }

    if (fd >= MAX_OPEN_FILES || table[proc][fd].fd == -1) {
        return -1;
    }

    // memset from util.h
    bzero(buf, count);

    current_inode = get_inode_per_inum(table[proc][fd].inode);

    // check if current inode is a file
    if ((current_inode.type == DIRECTORY && table[proc][fd].flag != FS_O_RDONLY) || table[proc][fd].flag == FS_O_WRONLY) {
        return -1;
    }

    // get current pointer position
    index_block = table[proc][fd].rw_ptr / super.block_size;
    rw = table[proc][fd].rw_ptr % super.block_size;

    int num_blocks = (current_inode.size + super.block_size - 1) / super.block_size;
    if (index_block >= num_blocks) {
        return 0;
    }

    iblock = get_iblock(current_inode, index_block);

    block_read(super.beg_data + iblock, (char *)&block);
    for (int i = 0; i < count; i++, rw++) {

        // if we already look throughout a block, we must load the next one
        if (rw == super.block_size) {
            rw = 0;
            iblock = get_iblock(current_inode, ++index_block);
            if (iblock == -1) {
                table[proc][fd].rw_ptr += i;
                return i;
            }
            block_read(super.beg_data + iblock, (char *)&block);
        }

        // check if we got the maximum size of the file
        if (current_inode.size <= table[proc][fd].rw_ptr + i) {
            table[proc][fd].rw_ptr += i;
            return i;
        }

        // save data to buf
        buf[i] = block.data[rw];
    }

    table[proc][fd].rw_ptr += count;
    return count;
}

int fs_write(int fd, char *buf, int count, int proc) {
    int rw, index_block, iblock, need;
    inode_t current_inode;
    DataBlock block;
    if (count == 0) {
        return 0;
    }

    if (fd >= MAX_OPEN_FILES || table[proc][fd].fd == -1) {
        return -1;
    }

    current_inode = get_inode_per_inum(table[proc][fd].inode);

    // check if file is a directory and R/W pointer is valid
    if (current_inode.type == DIRECTORY || table[proc][fd].flag == FS_O_RDONLY) {
        return -1;
    }

    need = table[proc][fd].rw_ptr - current_inode.size;

    if (need > 0) {
        index_block = current_inode.size / super.block_size;
        rw = current_inode.size % super.block_size;
    } else {
        need = 0;
        index_block = table[proc][fd].rw_ptr / super.block_size;
        rw = table[proc][fd].rw_ptr % super.block_size;
    }

    iblock = get_iblock(current_inode, index_block);
    if (iblock == -1) {
        if (index_block >= max_blocks_of_file() || blocks_used() == super.num_data_blocks) {
            return -1;
        }
        // try to allocate block
        iblock = get_single_available_iblock();
        if (iblock < 0) {
            return -1;
        }

        if (set_iblock(&current_inode, index_block, iblock) < 0) {
            free_iblock(iblock);
            return -1;
        }

        bzero((char *)&block, super.block_size);
        block_write(super.beg_data + iblock, (char *)&block);
    }

    block_read(super.beg_data + iblock, (char *)&block);
    for (int i = 0; i < count + need; i++, rw++) {
        if (rw == super.block_size) {
            // save block already written
            block_write(super.beg_data + iblock, (char *)&block);

            rw = 0;
            iblock = get_iblock(current_inode, ++index_block);
            if (iblock == -1) {
                // allocate if I can
                if (index_block >= max_blocks_of_file()) {
                    return -1;
                }
                iblock = get_single_available_iblock();
                if (iblock < 0) {
                    table[proc][fd].rw_ptr += i;
                    if (table[proc][fd].rw_ptr > current_inode.size) {
                        current_inode.size = table[proc][fd].rw_ptr;
                    }
                    save_inode(table[proc][fd].inode, current_inode); // save inode
                    save_map();
                    return i;
                }

                if (set_iblock(&current_inode, index_block, iblock) < 0) {
                    table[proc][fd].rw_ptr += i;
                    if (table[proc][fd].rw_ptr > current_inode.size) {
                        current_inode.size = table[proc][fd].rw_ptr;
                    }
                    save_inode(table[proc][fd].inode, current_inode); // save inode
                    free_inode(iblock);
                    save_map();
                    return i;
                }
            }
            block_read(super.beg_data + iblock, (char *)&block);
        }

        if (i < need) {
            block.data[rw] = 0;
        } else {
            block.data[rw] = buf[i - need];
        }
    }

    if (iblock >= 0) {
        block_write(super.beg_data + iblock, (char *)&block);
    }

    table[proc][fd].rw_ptr += count;
    if (table[proc][fd].rw_ptr > current_inode.size) {
        current_inode.size = table[proc][fd].rw_ptr;
    }
    save_inode(table[proc][fd].inode, current_inode); // save inode
    save_map();

    return count;
}

int fs_lseek(int fd, int offset, int proc) {
    if (fd >= MAX_OPEN_FILES || table[proc][fd].fd == -1) {
        return -1;
    }

    if (offset >= 0) {
        table[proc][fd].rw_ptr = offset;
        return offset;
    }

    return -1;
}

int fs_mkdir(char *fileName, int proc) {
    // check if dir with that name already exists
    inode_t parent_inode = get_inode_per_inum(current_dir[proc].files_inum[0]);
    if (find_file_in_dir(parent_inode, fileName, NULL) >= 0) {
        return -1;
    }
    // Allocate inode
    int inum = get_single_available_inode();
    if (inum < 0) {
        return -1;
    }

    // allocate data blocks
    int iblock = get_single_available_iblock();
    if (iblock < 0) {
        free_inode(inum);
        return -1;
    }

    // set dcb of the new directory
    dir_t new_dir = create_directory(inum);

    // set inode entries
    inode_t new_inode = (inode_t){
        .type = DIRECTORY,
        .link_counter = 1,
        .size = 2,
        .indirect1 = -1,
        .indirect2 = -1,
        .indirect3 = -1};
    for (int i = 0; i < super.direct_pointers; i++)
        new_inode.direct[i] = -1;
    new_inode.direct[0] = iblock;

    // update parent
    Block block;

    insert_file_in_dir(&parent_inode, fileName, inum);
    block_read(super.beg_data + parent_inode.direct[0], (char *)&block);
    current_dir[proc] = block.data_block.dir;

    parent_inode.size++;

    // write blocks to disk
    save_inode(inum, new_inode);                                // writing new inode
    save_inode(current_dir[proc].files_inum[0], parent_inode); // writing new inode

    block.data_block.dir = new_dir;
    block_write(super.beg_data + iblock, (char *)&block); // writing new data block

    block.map = map;
    block_write(MAP_BLOCK, (char *)&block); // writing bits map

    return 0;
}

int fs_rmdir(char *fileName, int proc) {
    inode_t parent_inode = get_inode_per_inum(current_dir[proc].files_inum[0]);
    // check if fileName exists
    int relIndex;
    int existFile = find_file_in_dir(parent_inode, fileName, &relIndex);
    if (existFile < 0) {
        return -1;
    }
    // check if directory is empty
    inode_t dir_inode = get_inode_per_inum(existFile);
    if (is_directory_empty(dir_inode) == FALSE) {
        return -1;
    }
    // remove subdirectory
    free_iblock(dir_inode.direct[0]); // free its only data block
    free_inode(existFile);            // free its inode number
    // remove link of parent dir to subdirectory
    remove_file_from_dir(&parent_inode, relIndex);
    parent_inode.size--;
    // load newest current dir
    Block aux;
    block_read(super.beg_data + parent_inode.direct[0], (char*)&aux);
    current_dir[proc] = aux.data_block.dir;
    // write to disk
    save_inode(current_dir[proc].files_inum[0], parent_inode); // save current dir inode
    save_map();
    return 0;
}


void update_path(char *dirName, int proc) {
    char *path = current_path[proc];
    int len = strlen(path);

    if (strcmp(dirName, ".") == 0) {
        return;
    }

    else if (strcmp(dirName, "..") == 0) {
        if (len <= 1) return; 

        int search_limit = len - 1;
        if (path[search_limit] == '/') {
            search_limit--; 
        }

        for (int i = search_limit; i >= 0; i--) {
            if (path[i] == '/') {
                if (i == 0) {
                    path[1] = '\0'; 
                } else {
                    path[i] = '\0';
                }
                break;
            }
        }
    }

    else {
        if (strcmp(path, "/") != 0) {
            strcat(path, "/");
        }
        strcat(path, dirName);
    }
}


int fs_cd(char *dirName, int proc) {
    DataBlock block;
    inode_t parent_inode = get_inode_per_inum(current_dir[proc].files_inum[0]);

    // check if fileName exists
    int existFile = find_file_in_dir(parent_inode, dirName, NULL);
    if (existFile < 0) {
        return -1;
    }

    // find fileName
    inode_t dir_inode = get_inode_per_inum(existFile);
    if (dir_inode.type == FILE_TYPE) {
        return -1;
    }

    // update current_dir for process
    int iblock = get_iblock(dir_inode, 0);
    block_read(super.beg_data + iblock, (char *)&block);

    current_dir[proc] = block.dir;
    update_path(dirName, proc);
    return 0;
}

int fs_link(char *old_fileName, char *new_fileName, int proc) {
    // check if old_fileName and new_fileName exists
    inode_t parent_inode = get_inode_per_inum(current_dir[proc].files_inum[0]);
    int old_inode = find_file_in_dir(parent_inode, old_fileName, NULL);
    if (old_inode < 0) {
        return -1;
    }

    // if new_fileName exists, finish
    int new_inode = find_file_in_dir(parent_inode, new_fileName, NULL);
    if (new_inode >= 0) {
        return -1;
    }

    // check old fileName is a FILE
    inode_t current_inode = get_inode_per_inum(old_inode);
    if (current_inode.type == DIRECTORY) {
        return -1;
    }

    // insert new_fileName on directory block
    if (insert_file_in_dir(&parent_inode, new_fileName, old_inode) < 0) {
        return -1;
    }
    parent_inode.size++;

    Block block;
    block_read(super.beg_data + parent_inode.direct[0], (char *)&block);
    current_dir[proc] = block.data_block.dir;

    // update inode of old_fileName on disk and memory
    // if its open
    current_inode.link_counter++;

    // save to disk
    save_inode(current_dir[proc].files_inum[0], parent_inode); // save changes in parent inode
    save_inode(old_inode, current_inode);                      // save changes in file inode

    return 0;
}

int fs_unlink(char *fileName, int proc) {
    // check if fileName exists
    inode_t parent_inode = get_inode_per_inum(current_dir[proc].files_inum[0]);
    int relIndex, fd;
    int file_inum = find_file_in_dir(parent_inode, fileName, &relIndex);
    if (file_inum < 0) {
        return -1;
    }
    // check if it is a directory
    inode_t current_inode = get_inode_per_inum(file_inum);
    if (current_inode.type == DIRECTORY) {
        return -1;
    }

    // remove link of parent dir to subdirectory
    remove_file_from_dir(&parent_inode, relIndex);
    parent_inode.size--;

    // update link counter of inode on disk and memory
    current_inode.link_counter--;

    for (fd = 0; fd < MAX_OPEN_FILES; fd++) {
        if (table[proc][fd].fd != -1 && same_string(table[proc][fd].name, fileName))
            break;
    }

    if (current_inode.link_counter == 0 && fd == MAX_OPEN_FILES) {
        // erase file

        // free all data blocks associate with this file
        free_all_data_blocks(current_inode);

        // free its inode
        free_inode(file_inum);
    } else {
        save_inode(file_inum, current_inode);
    }

    // load newest current dir
    Block aux;
    block_read(super.beg_data + parent_inode.direct[0], (char *)&aux);
    current_dir[proc] = aux.data_block.dir;

    // write to disk
    save_inode(current_dir[proc].files_inum[0], parent_inode); // save current dir inode

    save_map();

    return 0;
}

int fs_stat(char *fileName, fileStat *buf, int proc) {
    // get inode of parent
    inode_t parent_inode = get_inode_per_inum(current_dir[proc].files_inum[0]);

    // check if file exists
    int file_inum = find_file_in_dir(parent_inode, fileName, NULL);
    if (file_inum < 0) {
        return -1;
    }

    // get inode of fileName
    inode_t file_inode = get_inode_per_inum(file_inum);

    // set buf
    int num_blocks;
    if (file_inode.type == DIRECTORY) {
        num_blocks = (file_inode.size + super.pointers_per_dcb - 1) / super.pointers_per_dcb;
    } else {
        num_blocks = (file_inode.size + super.block_size - 1) / super.block_size;
    }
    *buf = (fileStat){
        .inodeNo = file_inum,
        .type = file_inode.type,
        .links = file_inode.link_counter,
        .size = file_inode.size,
        .numBlocks = num_blocks};
    return 0;
}

int fs_fsck(fsCheck *buf) {
    *buf = (fsCheck){
        .magic_number = super.magic_number,
        .inodes_allocated = inodes_used(),
        .blocks_allocated = blocks_used(),
        .map = map};
    return 0;
}