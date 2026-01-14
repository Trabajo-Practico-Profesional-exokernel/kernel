#ifndef INC_VIRT_BLK
#define INC_VIRT_BLK

#include "arch_inc/virtio.h"

typedef unsigned virt_blk_sector_t;
typedef uint64_t virt_blk_addr_t;



void virt_blk_queues_init(void);

void virtio_blk_init(void);


// int read_write_disk(void *buf, virt_blk_addr_t blk_addr, int is_write);
int read_write_disk(void *buf, virt_blk_sector_t blk_sector, int is_write);

#endif
