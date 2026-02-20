// Search for and parse the multiprocessor configuration table
// See https://datasheets.chipdb.org/Intel/x86/Intel%20Architecture/24201606.PDF

#include "arch_inc/mem_constants.h"
#include "arch_inc/trap_constants.h"
#include "arch_inc/x86.h"
 
#include "arch/cpus.h"
#include "assert.h"
#include "string.h"


struct cpu cpus[NCPU];
struct cpu *bootcpu;
int ismp;  				// true if mp detected
int ncpu;
uint8_t ioapicid;



// See MultiProcessor Specification Version 1.[14]

struct mp {             // floating pointer [MP 4.1]
	uint8_t signature[4];           // "_MP_"
	paddr_t physaddr;            // phys addr of MP config table
	uint8_t length;                 // 1
	uint8_t specrev;                // [14]
	uint8_t checksum;               // all bytes must add up to 0
	uint8_t type;                   // MP system config type
	uint8_t imcrp;
	uint8_t reserved[3];
} __attribute__((__packed__));

struct mpconf {         // configuration table header [MP 4.2]
	uint8_t signature[4];           // "PCMP"
	uint16_t length;                // total table length
	uint8_t version;                // [14]
	uint8_t checksum;               // all bytes must add up to 0
	uint8_t product[20];            // product id
	paddr_t oemtable;            // OEM table pointer
	uint16_t oemlength;             // OEM table length
	uint16_t entry;                 // entry count
	paddr_t lapicaddr;           // address of local APIC
	uint16_t xlength;               // extended table length
	uint8_t xchecksum;              // extended table checksum
	uint8_t reserved;
	uint8_t entries[0];             // table entries
} __attribute__((__packed__));

struct mpproc {         // processor table entry [MP 4.3.1]
	uint8_t type;                   // entry type (0)
	uint8_t apicid;                 // local APIC id
	uint8_t version;                // local APIC version
	uint8_t flags;                  // CPU flags
	uint8_t signature[4];           // CPU signature
	uint32_t feature;               // feature flags from CPUID instruction
	uint8_t reserved[8];
} __attribute__((__packed__));

struct mpioapic {       // I/O APIC table entry
	uint8_t type;                   // entry type (2)
	uint8_t apicno;                 // I/O APIC id
	uint8_t version;                // I/O APIC version
	uint8_t flags;                  // I/O APIC flags
	paddr_t *addr;                  // I/O APIC address
} __attribute__((__packed__));

// mpproc flags
#define MPPROC_BOOT 0x02                // This mpproc is the bootstrap processor

// Table entry types
#define MPPROC    0x00  // One per processor
#define MPBUS     0x01  // One per bus
#define MPIOAPIC  0x02  // One per I/O APIC
#define MPIOINTR  0x03  // One per bus interrupt source
#define MPLINTR   0x04  // One per system interrupt source


static uint8_t 
chksum(uint8_t *addr, int len)
{
  	int sum = 0;
  	for(int i = 0; i < len; i++)
  		sum += addr[i];
	return sum;
}


// Look for an MP structure in the len bytes at physical address addr.
static struct mp *
mpsearch1(paddr_t addr, int len)
{
	struct mp *mp = (struct mp *)(addr);
	struct mp *end = (struct mp *)(addr  + len);

	for (; mp < end; mp++)
		if (memcmp(mp->signature, "_MP_", 4) == 0 &&
		    chksum(mp, sizeof(*mp)) == 0) {
			return mp;
		}
	return NULL;
}

// Search for the MP Floating Pointer Structure, which according to
// [Chapter 4: MP Configuration Table] is in one of the following 
// three locations:
// 1) in the first KB of the Extended BIOS Data Area;
// 2) if there is no EBDA, in the last KB of system base memory;
// 3) in the BIOS ROM between 0xF0000 and 0xFFFFF.
static struct mp *
mpsearch(void)
{
	uint8_t *bda;
	uint32_t p;
	struct mp *mp;

	static_assert(sizeof(*mp) == 16);

	// The BIOS data area lives in 16-bit segment 0x40.
	bda = (uint8_t *) (0x40 << 4);

	// [MP 4] The 16-bit segment of the EBDA is in the two bytes
	// starting at byte 0x0E of the BDA.  0 if not present.
	if ((p = *(uint16_t *) (bda + 0x0E))) {
		p <<= 4;	// Translate from segment to PA
		if ((mp = mpsearch1(p, 1024)))
			return mp;
	} else {
		// The size of base memory, in KB is in the two bytes
		// starting at 0x13 of the BDA.
		p = *(uint16_t *) (bda + 0x13) * 1024;
		if ((mp = mpsearch1(p - 1024, 1024)))
			return mp;
	}
	return mpsearch1(0xF0000, 0x10000);
}


// Search for an MP configuration table.  For now, don't accept the
// default configurations (physaddr == 0).
// Check for the correct signature, checksum, and version.
static struct mpconf *
mpconfig(struct mp **pmp)
{
	struct mpconf *conf;
	struct mp *mp;

	if ((mp = mpsearch()) == 0)
		return NULL;

	if (mp->physaddr == 0 || mp->type != 0) {
		printf("SMP: Default configurations not implemented\n");
		return NULL;
	}
	conf = (struct mpconf *) (mp->physaddr);

	if (memcmp(conf, "PCMP", 4) != 0) {
		printf("SMP: Incorrect MP configuration table signature\n");
		return NULL;
	}

	if (chksum(conf, conf->length) != 0) {
		printf("SMP: Bad MP configuration checksum\n");
		return NULL;
	}

	if (conf->version != 1 && conf->version != 4) {
		printf("SMP: Unsupported MP version %d\n", conf->version);
		return NULL;
	}

	if ((chksum((uint8_t *)conf + conf->length, conf->xlength) + conf->xchecksum) & 0xff) {
		printf("SMP: Bad MP configuration extended checksum\n");
		return NULL;
	}
	*pmp = mp;
	return conf;
}

void init_cpus(void)
{
	struct mp *mp;
	struct mpconf *conf;
	struct mpproc *proc;
	struct mpioapic *ioapic;
	uint8_t *p;
	unsigned int i;

	bootcpu = &cpus[0]; // set default boot cpu 

	if ((conf = mpconfig(&mp)) == 0)
		return;
	ismp = 1;
	lapicaddr = conf->lapicaddr;

	for (p = conf->entries, i = 0; i < conf->entry; i++) {
		switch (*p) {
		case MPPROC:
			proc = (struct mpproc *)p;

			if (proc->flags & MPPROC_BOOT)
				bootcpu = &cpus[ncpu];

			if (ncpu < NCPU) {
				cpus[ncpu].cpu_apicid = proc->apicid;
				cpus[ncpu].cpu_id = ncpu; 
				cpus[ncpu].cpu_status = CPU_HALTED;
				ncpu++;
			} else {
				printf("SMP: too many CPUs, CPU %d disabled\n",
					proc->apicid);
			}
			p += sizeof(struct mpproc);
			continue;
		case MPBUS:
		case MPIOAPIC:
    		printf("SMP: IOAPIC detected\n");
      		ioapic = (struct mpioapic*)p;
      		ioapicid = ioapic->apicno;
      		p += sizeof(struct mpioapic);
      		continue;
		case MPIOINTR:
		case MPLINTR:
			p += 8;
			continue;
		default:
			printf("mpinit: unknown config type %x\n", *p);
			ismp = 0;
			i = conf->entry;
		}
	}
	bootcpu->cpu_status = CPU_STARTED;

	if (!ismp) {
		// Didn't like what we found; fall back to no MP.
		ncpu = 1;
		lapicaddr = 0;
		printf("SMP: configuration not found, SMP disabled\n");
		return;
	}
	printf("SMP: CPU %d found %d CPU(s)\n", bootcpu->cpu_id, ncpu);

	if (mp->imcrp) {
		// [MP 3.2.6.1] If the hardware implements PIC mode,
		// switch to getting interrupts from the LAPIC.
		printf("SMP: Setting IMCR to switch from PIC mode to symmetric I/O mode\n");
		outb(0x22, 0x70);   // Select IMCR
		outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
	}
}

