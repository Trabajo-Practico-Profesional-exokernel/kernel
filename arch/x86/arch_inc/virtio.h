#ifndef INC_VIRTIO
#define INC_VIRTIO

#include "inc/types.h"
#include "arch_inc/mem_constants.h"


/// Esta addr es tal ya que en qemu se maneja que los devices empiezen en 0x1000
/// Y despues cada uno ocupa 4096 i.e 1 pagina, osea VIRTIO BLK seria el segundo device!
extern uint32_t phys_base;

#define VIRTIO_BLK_PADDR (phys_base)

#define SECTOR_SIZE       512
#define VIRTQ_ENTRY_NUM   16
#define VIRTIO_DEVICE_BLK 2
#define VIRTIO_REG_MAGIC         0x00
#define VIRTIO_REG_VERSION       0x04
#define VIRTIO_REG_DEVICE_ID     0x08
#define VIRTIO_REG_QUEUE_SEL     0x30
#define VIRTIO_REG_QUEUE_NUM_MAX 0x34
#define VIRTIO_REG_QUEUE_NUM     0x38
#define VIRTIO_REG_QUEUE_ALIGN   0x3c
#define VIRTIO_REG_QUEUE_PFN     0x40
#define VIRTIO_REG_QUEUE_READY   0x44
#define VIRTIO_REG_QUEUE_NOTIFY  0x50
#define VIRTIO_REG_DEVICE_STATUS 0x70
#define VIRTIO_REG_DEVICE_CONFIG 0x100
#define VIRTIO_STATUS_ACK       1
#define VIRTIO_STATUS_DRIVER    2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEAT_OK   8
#define VIRTQ_DESC_F_NEXT          1
#define VIRTQ_DESC_F_WRITE         2
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
#define VIRTIO_BLK_T_IN  0
#define VIRTIO_BLK_T_OUT 1

#define VIRTIO_PCI_HOST_FEATURES   0x00  // 32-bit
#define VIRTIO_PCI_GUEST_FEATURES  0x04  // 32-bit
#define VIRTIO_PCI_QUEUE_PFN       0x08  // 32-bit
#define VIRTIO_PCI_QUEUE_NUM       0x0C  // 16-bit (RO)
#define VIRTIO_PCI_QUEUE_SEL       0x0E  // 16-bit
#define VIRTIO_PCI_QUEUE_NOTIFY    0x10  // 16-bit
#define VIRTIO_PCI_STATUS          0x12  // 8-bit
#define VIRTIO_PCI_ISR             0x13  // 8-bit (RO)

#define MMIO8(base, off)  (*(volatile uint8_t  *)((base) + (off)))
#define MMIO16(base, off) (*(volatile uint16_t *)((base) + (off)))
#define MMIO32(base, off) (*(volatile uint32_t *)((base) + (off)))



// Virtqueue Descriptor area entry.
struct virtq_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

// Virtqueue Available Ring.
struct virtq_avail {
    uint16_t flags;
    uint16_t index;
    uint16_t ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// Virtqueue Used Ring entry.
struct virtq_used_elem {
    uint32_t id;
    uint32_t len;
} __attribute__((packed));

// Virtqueue Used Ring.
struct virtq_used {
    uint16_t flags;
    uint16_t index;
    struct virtq_used_elem ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// Virtqueue.
struct virtio_virtq {
    struct virtq_desc descs[VIRTQ_ENTRY_NUM];
    struct virtq_avail avail;
    struct virtq_used used __attribute__((aligned(PAGE_SIZE)));
    int queue_index;
    volatile uint16_t *used_index;
    uint16_t last_used_index;
} __attribute__((packed));

// Virtio-blk request.
struct virtio_blk_req {
    uint32_t type;
    uint32_t reserved;
    uint64_t sector;
    uint8_t data[512];
    uint8_t status;
} __attribute__((packed));







uint32_t virtio_reg_read32(unsigned offset);
uint64_t virtio_reg_read64(unsigned offset);
void virtio_reg_write32(unsigned offset, uint32_t value);
void virtio_reg_fetch_and_or32(unsigned offset, uint32_t value);


void virtio_init(void);

struct virtio_virtq *virtio_queue_init(unsigned index);


void virtq_kick(struct virtio_virtq *vq, int desc_index);
bool virtq_is_busy(struct virtio_virtq *vq);

#endif /* !*/
