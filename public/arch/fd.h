#ifndef FD
#define FD

#include "inc/types.h"

#define MAX_SIZE_PATH 255
#define FD_PERM_READ  0x1
#define FD_PERM_WRITE 0x2
#define MAX_BUFFER_BYTES 256

typedef enum {
    FD_TYPE_NONE = 0,
    FD_TYPE_PIPE,
    FD_TYPE_IO
} fd_type_t;

typedef enum {
    OFF = 0,
    ON  = 1
} file_state_t;

struct File {
    file_state_t state; //estado actual del fd que marca si esta siendo usado o no
    fd_type_t type; // tipo de fd
    uint8_t perms; //permisos lectura o escritura
    uint8_t buffer[MAX_BUFFER_BYTES]; //informacion que contiene el fd
    uint8_t len_buffer; //numero que indican los bytes cargados al fd
    uint8_t index_readed; //indice bytes leidos
    uint8_t index_readedwrited; //indice bytes escritos
    file_state_t readopen; //estado extremo escritura
    file_state_t writeopen; //estado extremo lectura
};

#endif
