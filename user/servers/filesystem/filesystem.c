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

int fs_mkdir(char *filepath){
    printf("new path name: %s\n", filepath);

    // --------------------------------------------------------
    // 1. ASIGNAR UN INODO LIBRE (Scan IMAP)
    // --------------------------------------------------------
    Block imap;
    block_read(IMAP_POSITION, (char *)&imap);
    int new_inum = -1;

    // Buscar primer bit en 0 dentro de los inodos disponibles
    for(int i = 0; i < super.num_inodes; i++){
        // Verificar bit
        int byte = i / 8;
        int bit  = i % 8;
        if( !((imap.raw.data[byte] >> bit) & 1) ){
            // Encontrado libre: Marcar como ocupado
            imap.raw.data[byte] |= (1 << bit);
            new_inum = i;
            break;
        }
    }

    if(new_inum == -1){
        printf("Error: No free inodes.\n");
        return -1;
    }
    block_write(IMAP_POSITION, (char *)&imap); // Guardar cambio en IMAP

    // --------------------------------------------------------
    // 2. ASIGNAR UN BLOQUE DE DATOS LIBRE (Scan DMAP)
    // --------------------------------------------------------
    Block dmap;
    block_read(DMAP_POSITION, (char *)&dmap);
    int new_data_blk_idx = -1;

    for(int i = 0; i < super.num_data_blocks; i++){
        int byte = i / 8;
        int bit  = i % 8;
        if( !((dmap.raw.data[byte] >> bit) & 1) ){
            dmap.raw.data[byte] |= (1 << bit);
            new_data_blk_idx = i;
            break;
        }
    }

    if(new_data_blk_idx == -1){
        printf("Error: No free data blocks.\n");
        return -1;
    }
    block_write(DMAP_POSITION, (char *)&dmap); // Guardar cambio en DMAP

    int phys_block_pos = FIRST_DATA_BLOCK_POSITION + new_data_blk_idx;

    // --------------------------------------------------------
    // 3. INICIALIZAR EL CONTENIDO DEL NUEVO DIRECTORIO (. y ..)
    // --------------------------------------------------------
    Block data_blk;
    memset(&data_blk, 0, BLOCK_SIZE);

    // Entrada "."
    data_blk.dirents[0].inode = new_inum;
    strcpy(data_blk.dirents[0].name, ".");

    // Entrada ".." (Apunta a ROOT porque dijiste "dentro de root")
    data_blk.dirents[1].inode = 0; 
    strcpy(data_blk.dirents[1].name, "..");

    block_write(phys_block_pos, (char *)&data_blk);

    // --------------------------------------------------------
    // 4. CREAR Y GUARDAR EL NUEVO INODO
    // --------------------------------------------------------
    inode_t new_inode;
    new_inode.type = T_DIR;     // Es un directorio
    new_inode.link_counter = 1; 
    new_inode.size = 64;        // 2 entradas iniciales (. y ..)
    memset(new_inode.direct, 0, sizeof(new_inode.direct));
    new_inode.direct[0] = phys_block_pos;

    // Calcular posición del inodo en disco y guardarlo
    Block inode_blk;
    int inode_block_idx = FIRST_INODE_POSITION + (new_inum / INODES_PER_BLOCK);
    int inode_offset    = new_inum % INODES_PER_BLOCK;

    block_read(inode_block_idx, (char *)&inode_blk);
    inode_blk.inodes[inode_offset] = new_inode;
    block_write(inode_block_idx, (char *)&inode_blk);

    // --------------------------------------------------------
    // 5. AGREGAR LA ENTRADA AL DIRECTORIO RAÍZ (ROOT)
    // --------------------------------------------------------
    inode_t root_inode;
    inode_load(0, &root_inode); // Cargar Root (Inum 0)

    // Asumimos que Root tiene espacio en su primer bloque directo
    // (Simplificación solicitada)
    int root_phys_block = root_inode.direct[0];
    
    Block root_data;
    block_read(root_phys_block, (char *)&root_data);

    // Buscar el primer slot vacío en el bloque de datos del Root
    int entries_limit = BLOCK_SIZE / sizeof(dirent_t);
    int inserted = 0;

    for(int i = 0; i < entries_limit; i++){
        if(strlen(root_data.dirents[i].name) == 0){
            // Slot vacío encontrado
            root_data.dirents[i].inode = new_inum;
            strcpy(root_data.dirents[i].name, filepath);
            inserted = 1;
            break;
        }
    }

    if(inserted){
        // Guardar bloque de datos del Root actualizado
        block_write(root_phys_block, (char *)&root_data);

        // Actualizar tamaño del Root y guardarlo
        root_inode.size += sizeof(dirent_t);
    
        block_read(FIRST_INODE_POSITION, (char *)&inode_blk);
        inode_blk.inodes[0] = root_inode;
        block_write(FIRST_INODE_POSITION, (char *)&inode_blk);
        
        printf("Directory '%s' created successfully in Root (inum %d)\n", filepath, new_inum);
        print_filesystem();
        return 0;
    } else {
        printf("Error: Root directory is full (simplified version limit).\n");
        return -1;
    }
    
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