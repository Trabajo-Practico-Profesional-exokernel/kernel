#ifndef IDE_H
#define IDE_H

#include "types.h"

int ide_read(void *buf, uint32_t sector, size_t sz);
int ide_write(void *buf, uint32_t sector, size_t sz);
void ide_init(void);

#endif

