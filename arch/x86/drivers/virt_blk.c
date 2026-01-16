#include "arch_inc/virtio.h"
#include "arch_inc/virtio_blk.h"
#include "arch_inc/x86.h"
#include "arch/mem.h"

#include "inc/common.h"
#include "std/string.h"

struct virtio_virtq *blk_request_vq;
struct virtio_blk_req *blk_req;

paddr_t blk_req_paddr;
uint64_t blk_capacity;



#define VIRT_BLK_INDEX 0

// Queue that will be used for requests of virt_blk
void virt_blk_queues_init(void){
    // This just inits the queue for IO requests with virt_blk
    blk_request_vq = virtio_queue_init(VIRT_BLK_INDEX); // index =0 means is first queue
}

// Get info about size and alloc data
void virtio_blk_init() {
    // Get the disk capacity.
    blk_capacity = virtio_reg_read64(VIRTIO_PCI_QUEUE_NUM) * SECTOR_SIZE;
    
    printf("+ virtio-blk: capacity is %d bytes\n", (int)blk_capacity);

    // Allocate a region to store requests to the device... round up size of blk
    blk_req_paddr = alloc_pages(align_up(sizeof(*blk_req), PAGE_SIZE) / PAGE_SIZE);

    blk_req = (struct virtio_blk_req *) blk_req_paddr;	
}

/* Reads/writes from/to virtio-blk device.

    1. Construct a request in blk_req. Specify the sector number you want to access and the type of read/write.
    2. Construct a descriptor chain pointing to each area of blk_req.
    3. Add the index of the head descriptor of the descriptor chain to the Available Ring.
    4. Notify the device that there is a new pending request.
    5. Wait until the device finishes processing (aka busy-waiting or polling).
    6. Check the response from the device.
*/

int read_write_disk(void *buf, virt_blk_sector_t sector, int is_write){

    if (sector >= blk_capacity / SECTOR_SIZE) {
        printf("virtio: tried to read/write sector=%d, but capacity is %d\n",
              sector, blk_capacity / SECTOR_SIZE);
        return -1;
    }

    // Construct the virtqueue descriptors (using 3 descriptors).
    struct virtio_virtq *vq = blk_request_vq;

  /* Construir request */
    blk_req->type = is_write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN;
    blk_req->reserved = 0;
    blk_req->sector = sector;
//    blk_req->status = 0xFF;

    if (is_write)
        memcpy(blk_req->data, buf, SECTOR_SIZE);

    /* Descriptor 0: header */
    vq->descs[0].addr  = blk_req_paddr;
    vq->descs[0].len   = offsetof(struct virtio_blk_req, data);
    vq->descs[0].flags = VIRTQ_DESC_F_NEXT;
    vq->descs[0].next  = 1;

    /* Descriptor 1: data */
    vq->descs[1].addr  = blk_req_paddr + offsetof(struct virtio_blk_req, data);
    vq->descs[1].len   = SECTOR_SIZE;
    vq->descs[1].flags = VIRTQ_DESC_F_NEXT |
                          (is_write ? 0 : VIRTQ_DESC_F_WRITE);
    vq->descs[1].next  = 2;

    /* Descriptor 2: status */
    vq->descs[2].addr  = blk_req_paddr + offsetof(struct virtio_blk_req, status);
    vq->descs[2].len   = sizeof(uint8_t);
    vq->descs[2].flags = VIRTQ_DESC_F_WRITE;
    vq->descs[2].next  = 0;

    /* Publicar en avail */
    uint16_t avail_idx = vq->avail.index % VIRTQ_ENTRY_NUM;
    vq->avail.ring[avail_idx] = 0;

    __sync_synchronize();   /* barrera obligatoria */

    vq->avail.index++;

    /* Notificar device */
    virtio_reg_write32(VIRTIO_PCI_QUEUE_NOTIFY, vq->queue_index);

    /* Esperar completado */
    while (vq->last_used_index == vq->used.index)
        ;

    /* Consumir used ring */
    struct virtq_used_elem *e =
        &vq->used.ring[vq->last_used_index % VIRTQ_ENTRY_NUM];

    /* e->id == head descriptor (0) */
    vq->last_used_index++;

    __sync_synchronize();
    

    // virtio-blk: If a non-zero value is returned, it's an error.
    if (blk_req->status != 0) {
        printf("virtio: warn: failed to read/write sector=%d status=%d\n",
               sector, blk_req->status);
        return -2;
    }

    // For read operations, copy the data into the buffer.
    if (!is_write)
        memcpy(buf, blk_req->data, SECTOR_SIZE);
    return 0;
}
