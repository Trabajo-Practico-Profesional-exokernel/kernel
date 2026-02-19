#include "ide.h"
#include "arch_inc/x86.h"
#include "constants.h"

#include "string.h"
#include "stdlib.h"


#define SECTOR_SIZE   512
#define IDE_ERR       0x01
#define IDE_DRQ	      0x08
#define IDE_SRV	      0x0F
#define IDE_DF        0x20
#define IDE_DRDY      0x40
#define IDE_BSY       0x80

#define IDE_CMD_RD    0x20
#define IDE_CMD_WR	  0x30
#define IDE_CMD_RDMUL 0xC4 	 // perform multiple reads
#define IDE_CMD_WRMUL 0xC5   // perform multiple writes

/* 		IDE ports		*/
#define P_DATA	  	  0x1F0  
#define P_ERR	  	  0x1F1  // error
#define P_FEAT	  	  0x1F1	 // features
#define P_NSECT	      0x1F2  // sector count
#define P_LBA_LO	  0x1F3  // lba low
#define P_LBA_MID	  0x1F4  // lba mid
#define P_LBA_HI      0x1F5  // lba high
#define P_DRV         0x1F6  // drive/head selector
#define P_STATUS	  0x1F7
#define P_CMD   	  0x1F7

#define MAX_SECT_OP	  256	 // max batch size operation

#define B2SEC(bsz) (((bsz) + SECTOR_SIZE - 1) / SECTOR_SIZE)


static int disk = 0;

int
ide_wait(bool check_err, bool wait_drq)
{
	uint8_t r;

	while (1) {
		r = inb(P_STATUS);
		if (!(r & IDE_BSY) && (r & IDE_DRDY) && (!wait_drq | r & IDE_DRQ)) 
			break;
	}

	if (check_err && (r & (IDE_ERR|IDE_DF)) != 0) {
		return -1;
	}
	return 0;
}

void
ide_set_disk(int d)
{
	if (d != 0 && d != 1) {
		PANIC("bad disk number");
	}
	disk = d;
}

// ins wrapper; performs the optimal no of ops
void 
ins(uint16_t port, void *buf, uint32_t bsz)
{
	// bsz = 4*dw + 2*w + b
	uint32_t dw, w, b;
	dw = bsz >> 2;  		// bsz / 4
	w  = (bsz & 0x3) >> 1;  // remainder / 2
	b  = bsz & 0x1;			 

	if (dw) {
		insl(port, buf, dw);
		buf += (dw << 2);  // 4*dw
	} 

	if (w) {
		insw(port, buf, w);
		buf += (w << 1);   // 2*w
	}

	if (b) {
		insb(port, buf, b);
	}
}

// outs wrapper; performs the optimal no of ops
void
outs(uint16_t port, void *buf, uint32_t bsz)
{
	// bsz = 4*dw + 2*w + b
	uint32_t dw, w, b;
	dw = bsz >> 2;  		// bsz / 4
	w  = (bsz & 0x3) >> 1;  // remainder / 2
	b  = bsz & 0x1;			 

	if (dw) {
		outsl(port, buf, dw);
		buf += (dw << 2);  // 4*dw
	} 

	if (w) {
		outsw(port, buf, w);
		buf += (w << 1);   // 2*w
	}

	if (b) {
		outsb(port, buf, b);
	}
}

int
ide_read(void *buf, uint32_t sector, size_t nsecs)
{
	if (nsecs < 0 || nsecs >= MAX_SECT_OP)
		return -1;
		
	ide_wait(0, 0);
	
	outb(P_NSECT, nsecs);

	// LBA28 defines a logical bit address of 28 bits
	// first 24 bits of LBA
	outb(P_LBA_LO, sector & 0xFF);			
	outb(P_LBA_MID, (sector >> 8) & 0xFF);	
	outb(P_LBA_HI, (sector >> 16) & 0xFF);	

	// bit 24 to 27 of lba + disk number
	// Drive / Head register:
	// | 1 | LBA=1 | 1 | DRV | LBA 24~27 |
	//   7     6     5    4       0~3
	outb(P_DRV, 0xE0 | ((disk & 1) << 4) | ((sector >> 24) & 0x0F));

	// send command
	outb(P_CMD, nsecs > 1 ? IDE_CMD_RDMUL : IDE_CMD_RD);
	
	// fill the buffer	
	for (; nsecs > 0; nsecs--, buf+=SECTOR_SIZE) {
		if (ide_wait(1, 0) < 0)
			return -1;

		insl(P_DATA, buf, SECTOR_SIZE / 4);	
	}
	return 0;
}

//  Write operation.
//  Always writes sector aligned. The remaining bytes are filled with zeroes.
//  e.g. sz = 600B  -> sector 1 = buf[0...512];  sector 2 = buf[512...599] + 424 zeroes
int
ide_write(void *buf, uint32_t sector, size_t nsecs)
{
	if (nsecs < 0 || nsecs >= MAX_SECT_OP)
		return -1;
		
	ide_wait(0, 0);
	
	outb(P_NSECT, nsecs);

	// LBA28 defines a logical bit address of 28 bits
	// first 24 bits of LBA
	outb(P_LBA_LO, sector & 0xFF);			
	outb(P_LBA_MID, (sector >> 8) & 0xFF);	
	outb(P_LBA_HI, (sector >> 16) & 0xFF);	

	// bit 24 to 27 of lba + disk number
	// Drive / Head register:
	// | 1 | LBA=1 | 1 | DRV | LBA 24~27 |
	//   7     6     5    4       0~3
	outb(P_DRV, 0xE0 | ((disk & 1) << 4) | ((sector >> 24) & 0x0F));

	// send command
	outb(P_CMD, nsecs > 1 ? IDE_CMD_WRMUL : IDE_CMD_WR);
	
	// fill the buffer	
	for (; nsecs > 0; nsecs--, buf+=SECTOR_SIZE) {
		if (ide_wait(1, 0) < 0)
			return -1;

		outsl(P_DATA, buf, SECTOR_SIZE / 4);	
	}
	return 0;
}


bool
ide_probe_disk1()
{
	int i;
	ide_wait(0, 0);

	// switch to dev 1
	outb(P_DRV, 0xE0 | (1 << 4));

	// check if ready
	for (i = 0; 
		 i < 1000 && (inb(P_STATUS) & (IDE_BSY|IDE_DF|IDE_ERR)) != 0;
		 i++) {
		/* nop */
	}
	
	// switch back to dev 0
	outb(P_DRV, 0xE0 | (0 << 4));
	printf("+ device 1 found: %d\n", i < 1000);
	return i < 1000;
}


void
ide_init()
{
	// use dev 1 if available
	ide_set_disk((ide_probe_disk1() ? 1 : 0));
}

