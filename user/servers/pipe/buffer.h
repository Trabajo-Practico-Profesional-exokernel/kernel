#ifndef BUFFER_H
#define BUFFER_H

#define MAX_BUFFER_BYTES 256

#include "types.h"

struct MemBuffer{
    uint8_t references; //indica cuandos files tienen el buffer tomado
    uint8_t buffer[MAX_BUFFER_BYTES]; //informacion que contiene el fd
    uint8_t len_buffer; //numero que indican los bytes cargados
    uint8_t write_idx; //indice bytes leidos
    uint8_t read_idx; //indice bytes escritos
};

void init_membuffers();

void membuffer_reset(struct MemBuffer *buffer);

struct MemBuffer* membuffer_alloc(void);

void membuffer_release(struct MemBuffer *mb);

int membuffer_write(struct MemBuffer *mb, const uint8_t *src, uint8_t len);

int membuffer_read(struct MemBuffer *mb, uint8_t *dst, uint8_t len);

int membuffer_is_full(struct MemBuffer *mb);

int membuffer_is_empty(struct MemBuffer *mb);

#endif
