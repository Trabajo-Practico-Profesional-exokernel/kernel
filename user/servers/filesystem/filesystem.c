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

    // check if disk is formatted
    if(block.super_block.magic_number == MAGIC_NUMBER){
        super = block.super_block; // set superblock
        printf("Superblock formatted!\n");
    }else{
        fs_mkfs(); // format disk
    }
}

// Asegúrate de tener estas definiciones en tu header o arriba
#define T_DIR  2
#define T_FILE 1




// --- Funciones Auxiliares Necesarias ---

// Carga un inodo desde el disco a una estructura en memoria
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

// Función recursiva para recorrer el árbol
// inum: inodo del directorio o archivo a procesar
// level: nivel de profundidad (para la indentación)
void fs_tree_recursive(uint32_t inum, int level) {
    inode_t inode;
    inode_load(inum, &inode);

    // Si no es un directorio, no hay nada más que recorrer (caso base implícito)
    if (inode.type != T_DIR) {
        return;
    }

    // Recorrer los bloques de datos del directorio
    for (int i = 0; i < DIRECT_POINTERS; i++) {
        int phys_block = inode.direct[i];
        
        // Si el puntero es 0, no hay bloque asignado
        if (phys_block == 0) continue;

        Block block;
        block_read(phys_block, (char *)&block);

        // Iterar sobre las entradas (dirents) dentro del bloque
        // En 512 bytes caben 16 entradas (512 / 32)
        int entries_count = BLOCK_SIZE / sizeof(dirent_t);

        for (int j = 0; j < entries_count; j++) {
            // Validar si la entrada está vacía (nombre de longitud 0)
            if (strlen(block.dirents[j].name) == 0) continue;

            char *name = block.dirents[j].name;
            uint32_t child_inum = block.dirents[j].inode;

            // IMPORTANTE: Saltar "." y ".." para evitar bucles infinitos [cite: 264-265]
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }

            // 1. Imprimir la indentación visual
            for (int k = 0; k < level; k++) {
                printf("    "); // 4 espacios por nivel
            }
            
            // 2. Imprimir el nombre del archivo/directorio actual
            printf("|-- %s\n", name);

            // 3. Llamada Recursiva
            // Verificamos si este hijo es un directorio para profundizar
            inode_t child_inode;
            inode_load(child_inum, &child_inode);
            
            if (child_inode.type == T_DIR) {
                fs_tree_recursive(child_inum, level + 1);
            }
        }
    }
}

// --- Función Principal Solicitada ---

void print_filesystem(){
    printf("PRINTING FILESYSTEM (Tree View):\n");
    printf("/ (Root)\n"); // Imprimimos la raíz manualmente
    
    // Iniciar la recursión desde el Inodo 0 (Root) con nivel 0
    fs_tree_recursive(0, 0);
    
    printf("\n");
}

int fs_mkfs(void) {
    printf("formatting superblock!!\n");
    
    // 1. Limpiar todo el disco (poner ceros en los 64 bloques)
    for(int i = 0; i < FILESYSTEM_TOTAL_BLOCKS; i++){
        set_zero_block(i);
    }

    // Variable temporal para escrituras
    Block block;

    // ---------------------------------------------------------
    // 2. Inicializar y Escribir SUPERBLOQUE
    // ---------------------------------------------------------
    super.magic_number = MAGIC_NUMBER;
    super.size_disk = FILESYSTEM_TOTAL_BLOCKS * BLOCK_SIZE;
    super.block_size = BLOCK_SIZE;
    super.num_inodes = INODES_PER_BLOCK * INODE_BLOCKS; // 16 * 5 = 80
    super.num_data_blocks = DATA_REGION_BLOCKS;         // 56
    super.num_blocks_inodes = INODE_BLOCKS;
    
    // Escribir a disco
    block.super_block = super;
    block_write(SUPER_BLOCK_POSITION, (char *)&block);

    // ---------------------------------------------------------
    // 3. Inicializar Bitmaps (IMAP y DMAP)
    // ---------------------------------------------------------
    // Inicializamos las estructuras globales en memoria a 0
    memset(i_bmap.data, 0, BLOCK_SIZE);
    memset(d_bmap.data, 0, BLOCK_SIZE);

    // Reservamos el Inodo 0 para el ROOT
    // Bit 0 del byte 0 en 1: 00000001
    i_bmap.data[0] |= 1; 

    // Reservamos el Bloque de Datos 0 (físico 8) para los datos del ROOT
    d_bmap.data[0] |= 1;

    // Escribir IMAP a disco (Bloque 1)
    block.raw = i_bmap;
    block_write(IMAP_POSITION, (char *)&block);

    // Escribir DMAP a disco (Bloque 2)
    block.raw = d_bmap;
    block_write(DMAP_POSITION, (char *)&block);

    // ---------------------------------------------------------
    // 4. Crear el Inodo del Directorio Raíz (Inodo 0)
    // ---------------------------------------------------------
    // Leemos el bloque donde vive el inodo 0 (FIRST_INODE_POSITION = 3)
    // Aunque sabemos que está en cero, es buena práctica leer-modificar-escribir
    block_read(FIRST_INODE_POSITION, (char *)&block);

    // Configuramos el inodo 0
    block.inodes[0].type = T_DIR;
    block.inodes[0].size = 2 * sizeof(dirent_t); 
    block.inodes[0].link_counter = 1;
    block.inodes[0].direct[0] = FIRST_DATA_BLOCK_POSITION; 

    block_write(FIRST_INODE_POSITION, (char *)&block);

    // ---------------------------------------------------------
    // 5. Crear el contenido del Directorio Raíz (. y ..)
    // ---------------------------------------------------------
    // Preparamos el bloque de datos 8 (FIRST_DATA_BLOCK_POSITION)
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