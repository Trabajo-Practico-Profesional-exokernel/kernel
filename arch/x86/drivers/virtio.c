#include "arch_inc/virtio.h"
#include "arch_inc/virtio_blk.h"
#include "arch_inc/x86.h"

#include "inc/common.h"
#include "arch/mem.h"
#include "std/string.h"

#define CONFIG_ADDRESS 	0xCF8
#define CONFIG_DATA 	0xCFC

#define PCI_COMMAND_IO     (1 << 0)
#define PCI_COMMAND_MEM    (1 << 1)
#define PCI_COMMAND_MASTER (1 << 2)

uint32_t phys_base;
uint32_t bar0_size;

uint32_t virtio_reg_read32(unsigned offset) {
	return inl(VIRTIO_BLK_PADDR + offset);
    //return *((volatile uint32_t *) (VIRTIO_BLK_PADDR + offset));
}

uint64_t virtio_reg_read64(unsigned offset) {
	return inl(VIRTIO_BLK_PADDR + offset);
    //return *((volatile uint64_t *) (VIRTIO_BLK_PADDR + offset));
}

void virtio_reg_write32(unsigned offset, uint32_t value) {
	outl(VIRTIO_BLK_PADDR + offset, value);
    //*((volatile uint32_t *) (VIRTIO_BLK_PADDR + offset)) = value;
}

void virtio_reg_fetch_and_or32(unsigned offset, uint32_t value) {
    virtio_reg_write32(VIRTIO_BLK_PADDR + offset, virtio_reg_read32(VIRTIO_BLK_PADDR + offset) | value);
}

static inline uint32_t pci_config_read32(uint8_t bus, uint8_t dev, uint8_t fun, uint8_t off)
{
    uint32_t addr =
        (1U << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)dev << 11) |
        ((uint32_t)fun << 8)  |
        (off & 0xFC);

    outl(CONFIG_ADDRESS, addr);
 	return inl(CONFIG_DATA);
}

static inline void pci_set_addr(uint8_t bus, uint8_t dev, uint8_t fun, uint8_t off)
{
    uint32_t addr =
        (1U << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)dev << 11) |
        ((uint32_t)fun << 8)  |
        (off & 0xFC);

    outl(CONFIG_ADDRESS, addr);
}


static inline void pci_enable_device(uint8_t bus, uint8_t dev, uint8_t fun)
{
    uint32_t v = pci_config_read32(bus, dev, fun, 0x04);
    uint16_t cmd = v & 0xFFFF;

    cmd |= PCI_COMMAND_MEM;
    cmd |= PCI_COMMAND_MASTER;

    v = (v & 0xFFFF0000) | cmd;
    outl(CONFIG_ADDRESS,
        (1U << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)dev << 11) |
        ((uint32_t)fun << 8)  |
        (0x04 & 0xFC));
    outl(CONFIG_DATA, v);
}



/* TUTORIAL/PROCESS FROM VIRTIO SPEC!


The driver MUST follow this sequence to initialize a device:

    1. Reset the device.
    2. Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.
    3. Set the DRIVER status bit: the guest OS knows how to drive the device.
    4. Read device feature bits, and write the subset of feature bits understood by the OS and driver to the device. During this step the driver MAY read (but MUST NOT write) the device-specific configuration fields to check that it can support the device before accepting it.
    5. Set the FEATURES_OK status bit. The driver MUST NOT accept new feature bits after this step.
    6. Re-read device status to ensure the FEATURES_OK bit is still set: otherwise, the device does not support our subset of features and the device is unusable.
    7. Perform device-specific setup, including discovery of virtqueues for the device, optional per-bus setup, reading and possibly writing the device’s virtio configuration space, and population of virtqueues.
    8. Set the DRIVER_OK status bit. At this point the device is “live”.

If any of these steps go irrecoverably wrong, the driver SHOULD set the FAILED status bit to indicate that it has given up on the device (it can reset the device later to restart if desired). The driver MUST NOT continue initialization in that case.

The driver MUST NOT send any buffer available notifications to the device before setting DRIVER_OK. 


 */
#define VIRTIO_MAGIC_NUMBER 0x74726976 // Number used for checking all ok/no corruption



void virtio_init(void) {
	static uint8_t virtio_blk_bus;
	static uint8_t virtio_blk_dev;
	static uint8_t virtio_blk_fun;

	// STEP 1: scan PCI (discovery)
	static int found = 0;

	for (uint8_t bus = 0; !found && bus < 256; bus++) {
		for (uint8_t dev = 0; !found && dev < 32; dev++) {
			for (uint8_t fun = 0; !found && fun < 8; fun++) {
				uint32_t id = pci_config_read32(bus, dev, fun, 0x00);
				if ((id & 0xFFFF) == 0xFFFF)
					continue;

				uint16_t vendor = id & 0xFFFF;
				uint16_t device = id >> 16;

				if (vendor == 0x1AF4 && device == 0x1001 && !found) {
						found = 1;
						printf("+ virtio legacy device found\n");
						virtio_blk_bus = bus;
						virtio_blk_dev = dev;
						virtio_blk_fun = fun;
					// candidato virtio
				}
			}
		}
	}

	// STEP 2: enable PCI device 
	if (!found) {
		PANIC("- no virtio device found");	
	}
	//printf("+ virtio device found\n");
    pci_enable_device(virtio_blk_bus, virtio_blk_dev, virtio_blk_fun);		

	// STEP 3: read and decode BAR0
	uint32_t bar0;
	uint32_t bar0_orig;

	/* 1. Leer BAR0 */
	pci_set_addr(virtio_blk_bus, virtio_blk_dev, virtio_blk_fun, 0x10);
	bar0 = inl(CONFIG_DATA);


	/* 3. Extraer base física */
	phys_base = bar0 & 0xFFFFFFF0;
	printf("+ phys base: 0x%x\n", phys_base);

	/* 4. Calcular tamaño del BAR */
	bar0_orig = bar0;

	pci_set_addr(virtio_blk_bus, virtio_blk_dev, virtio_blk_fun, 0x10);
	outl(CONFIG_DATA, 0xFFFFFFFF);

	pci_set_addr(virtio_blk_bus, virtio_blk_dev, virtio_blk_fun, 0x10);
	bar0_size = inl(CONFIG_DATA);

	pci_set_addr(virtio_blk_bus, virtio_blk_dev, virtio_blk_fun, 0x10);
	outl(CONFIG_DATA, bar0_orig);

	/* 5. Decodificar tamaño */
	bar0_size = ~(bar0_size & 0xFFFFFFF0) + 1;

	printf("+ device size: %d\n", bar0_size);
	/* Resultado:
	 * phys_base  → base física MMIO del virtio-pci
	 * bar0_size  → tamaño del bloque de registros
	 */
	// STEP 5: 
	void *virtio_mmio = phys_base;
	outb(virtio_mmio + VIRTIO_PCI_STATUS, 0);
		
	printf("+ device reset\n");
	uint8_t st = inb(virtio_mmio + VIRTIO_PCI_STATUS);

	if (st != 0)
	    PANIC("virtio-blk: reset fallido");

	// STEP 6: ACK
	outb(virtio_mmio + VIRTIO_PCI_STATUS, VIRTIO_STATUS_ACK);
    
	// STEP 7: DRIVER
	outb(virtio_mmio + VIRTIO_PCI_STATUS,
	     VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER);

    
	// STEP 8: FEATURES
    uint32_t devf = inl(virtio_mmio + VIRTIO_PCI_HOST_FEATURES);
	uint32_t guestf = 0;

	outl(virtio_mmio + VIRTIO_PCI_GUEST_FEATURES, guestf);

	outb(virtio_mmio + VIRTIO_PCI_STATUS,
		 VIRTIO_STATUS_ACK |
		 VIRTIO_STATUS_DRIVER |
		 VIRTIO_STATUS_FEAT_OK);
 

	st = inb(virtio_mmio + VIRTIO_PCI_STATUS);
	if (!(st & VIRTIO_STATUS_FEAT_OK))
		PANIC("virtio: FEATURES_OK rejected");
	printf("+ features ok\n");

    // Init queues for blk, done in virt_blk.c
	virt_blk_queues_init();
/*
	outw(virtio_mmio + VIRTIO_PCI_QUEUE_SEL, 0); // qid=0 means first queue

	uint16_t qsz = inw(virtio_mmio + VIRTIO_PCI_QUEUE_NUM);
	if (qsz == 0)
		PANIC("virtio-blk: queue not available");

	virtio_blk_init(virtio_mmio);
*/
    // 9. Set the DRIVER_OK status bit.

	outb(virtio_mmio + VIRTIO_PCI_STATUS,
		 VIRTIO_STATUS_ACK |
		 VIRTIO_STATUS_DRIVER |
		 VIRTIO_STATUS_FEAT_OK |
		 VIRTIO_STATUS_DRIVER_OK);

}

/*
Virt queue also needs to be initialized... 
which is used for sending the requests to do something and so on.
There might be more than 1
*/
struct virtio_virtq * virtio_queue_init(unsigned index) {
    
    // Allocate a region for the virtqueue.
    
    paddr_t virtq_paddr = alloc_pages(align_up(sizeof(struct virtio_virtq), PAGE_SIZE) / PAGE_SIZE);
    
    struct virtio_virtq *virtq_obj = (struct virtio_virtq *) virtq_paddr;
    
    virtq_obj->queue_index = index;
    virtq_obj->used_index = (volatile uint16_t *) &virtq_obj->used.index;
    
    // 1. Select the queue writing its index (first queue is 0) to QueueSel.
    virtio_reg_write32(VIRTIO_PCI_QUEUE_SEL, index);
    
    // 5. Notify the device about the queue size by writing the size to QueueNum.
	/* 2. leer tamaño (NO se escribe) */
	uint16_t qsz = virtio_reg_read32(VIRTIO_PCI_QUEUE_NUM);
	if (qsz == 0)
		PANIC("virtio: queue not present");
    
    // 7. Write the physical number of the first page of the queue to the QueuePFN register.
    virtio_reg_write32(VIRTIO_PCI_QUEUE_PFN, virtq_paddr >> 12);
    return virtq_obj;
}


// Finally some actions in a specified virtq

// Notifies the device that there is a new request. `desc_index` is the index
// of the head descriptor of the new request.
void virtq_kick(struct virtio_virtq *vq, int desc_index) {
    vq->avail.ring[vq->avail.index % VIRTQ_ENTRY_NUM] = desc_index;
    __sync_synchronize();

    vq->avail.index++;

    __sync_synchronize();

    virtio_reg_write32(VIRTIO_PCI_QUEUE_NOTIFY, vq->queue_index);
    vq->last_used_index++;
}

// Returns whether there are requests being processed by the device.
bool virtq_is_busy(struct virtio_virtq *vq) {
    return vq->last_used_index == *(vq->used_index);
}

