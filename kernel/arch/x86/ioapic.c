// The I/O APIC manages hardware interrupts for an SMP system.
// Since we dont use PIC (see picirq.c), and LAPIC does not manage I/O,
// we have to set up I/O APIC
// See:
//	- https://pdos.csail.mit.edu/6.828/2016/readings/ia32/ioapic.pdf

#include "arch_inc/cpu.h"
#include "arch_inc/mem_constants.h"

#include "trap.h"


#define IOREGSEL   0xFEC00000   // I/O register select (index)
#define IOWIN      0xFEC00010   // I/O window (data)

#define IOAPICID   0x00  // Register index: ID
#define IOAPICVER  0x01  // Register index: version
#define IOAPICARB  0x02  // Register index: version
#define IOREDTBL   0x10  // Redirection table base

// The redirection table starts at REG_TABLE and uses
// two registers to configure each interrupt.
// The first (low) register in a pair contains configuration bits.
// The second (high) register contains a bitmask telling which
// CPUs can serve that interrupt.
#define INT_DISABLED   0x00010000  // Interrupt disabled
#define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
#define INT_ACTIVELOW  0x00002000  // Active low (vs high)
#define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)


volatile struct ioapic *ioapic;

// IO APIC MMIO structure: write reg, then read or write data.
// Memory Mapped registers for accessing IOAPIC registers:
// 	IOREGSEL -> [0:7] register selector; [8:31] reserved
//  IOWIN    -> [0:31] data
struct ioapic {
	uint32_t reg;
	uint32_t pad[3];  // IOWIN - IOREGSEL - 1
	uint32_t data;
};

static uint32_t 
ioapicr(int reg)
{
	ioapic->reg = reg;
	return ioapic->data;
}

static void
ioapicw(int reg, uint32_t data)
{
	ioapic->reg = reg;
	ioapic->data = data;
}

void
ioapic_init(void)
{
	ioapic = (volatile struct ioapic *)IOAPIC_BASE;
	int maxintr = (ioapicr(IOAPICVER) >> 16) & 0xFF;
	//printf("MAXINTR %d\n", maxintr);
	int id = ioapicr(IOAPICID) >> 24;

	if(id != ioapicid)
		printf("ioapicinit: id isn't equal to ioapicid; not a MP\n");

	// Mark all interrupts edge-triggered, active high, disabled,
	// and not routed to any CPUs.
	for (int i = 0; i <= maxintr; i++){
		ioapicw(IOREDTBL + 2*i, INT_DISABLED | (T_IRQ0 + i));
		ioapicw(IOREDTBL + 2*i + 1, 0);
	}
}

void
ioapic_enable(int irq, int cpunum)
{
	// Mark interrupt edge-triggered, active high,
	// enabled, and routed to the given cpunum,
	// wich happens to be that cpu's APIC ID.
 	ioapicw(IOREDTBL + 2 * irq, T_IRQ0 + irq);
	ioapicw(IOREDTBL + 2 * irq + 1, cpunum << 24);
}

