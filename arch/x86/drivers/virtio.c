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


uint32_t virtio_reg_read32(unsigned offset) {
    return *((volatile uint32_t *) (VIRTIO_BLK_PADDR + offset));
}

uint64_t virtio_reg_read64(unsigned offset) {
    return *((volatile uint64_t *) (VIRTIO_BLK_PADDR + offset));
}

void virtio_reg_write32(unsigned offset, uint32_t value) {
    *((volatile uint32_t *) (VIRTIO_BLK_PADDR + offset)) = value;
}

void virtio_reg_fetch_and_or32(unsigned offset, uint32_t value) {
    virtio_reg_write32(offset, virtio_reg_read32(offset) | value);
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

uint32_t phys_base;
uint32_t bar0_size;


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
						printf("CANDIDATO LEGACY\n");
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
		PANIC("No virtio device found");	
	}
	printf("virtio device found");
    pci_enable_device(virtio_blk_bus, virtio_blk_dev, virtio_blk_fun);		

	// STEP 3: read and decode BAR0
	uint32_t bar0;
	uint32_t bar0_orig;
	uint32_t bar0_size;
	uint32_t phys_base;

	/* 1. Leer BAR0 */
	pci_set_addr(virtio_blk_bus, virtio_blk_dev, virtio_blk_fun, 0x10);
	bar0 = inl(CONFIG_DATA);


	/* 3. Extraer base física */
	phys_base = bar0 & 0xFFFFFFF0;

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

	/* Resultado:
	 * phys_base  → base física MMIO del virtio-pci
	 * bar0_size  → tamaño del bloque de registros
	 */
	// STEP 5: 
	void *virtio_mmio = phys_base;
	MMIO8(virtio_mmio, VIRTIO_PCI_STATUS) = 0;
		
	uint8_t st = MMIO8(virtio_mmio, VIRTIO_PCI_STATUS);
	if (st != 0)
	    PANIC("virtio-blk: reset fallido");

	printf("Paso 5 completo");

	// PASO 6
    // 1. Reset the device.
    virtio_reg_write32(VIRTIO_REG_DEVICE_STATUS, 0);

    // 2. Set the ACKNOWLEDGE status bit: the guest OS has noticed the device.
    virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_ACK);
    
    // 3. Set the DRIVER status bit.
    virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_DRIVER);
    
    // 5. Set the FEATURES_OK status bit.
    virtio_reg_fetch_and_or32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_FEAT_OK);
    
    // 7. Perform device-specific setup, including discovery of virtqueues for the device

    // Init queues for blk, done in virt_blk.c
    virt_blk_queues_init();


    // 8. Set the DRIVER_OK status bit.
    virtio_reg_write32(VIRTIO_REG_DEVICE_STATUS, VIRTIO_STATUS_DRIVER_OK);
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
    virtio_reg_write32(VIRTIO_REG_QUEUE_SEL, index);
    
    // 5. Notify the device about the queue size by writing the size to QueueNum.
    virtio_reg_write32(VIRTIO_REG_QUEUE_NUM, VIRTQ_ENTRY_NUM);
    
    // 6. Notify the device about the used alignment by writing its value in bytes to QueueAlign.
    virtio_reg_write32(VIRTIO_REG_QUEUE_ALIGN, 0);
    
    // 7. Write the physical number of the first page of the queue to the QueuePFN register.
    virtio_reg_write32(VIRTIO_REG_QUEUE_PFN, virtq_paddr);
    return virtq_obj;
}



// Finally some actions in a specified virtq

// Notifies the device that there is a new request. `desc_index` is the index
// of the head descriptor of the new request.
void virtq_kick(struct virtio_virtq *vq, int desc_index) {
    vq->avail.ring[vq->avail.index % VIRTQ_ENTRY_NUM] = desc_index;
    vq->avail.index++;

    __sync_synchronize();
    virtio_reg_write32(VIRTIO_REG_QUEUE_NOTIFY, vq->queue_index);
    vq->last_used_index++;
}

// Returns whether there are requests being processed by the device.
bool virtq_is_busy(struct virtio_virtq *vq) {
    return vq->last_used_index != *vq->used_index;
}

