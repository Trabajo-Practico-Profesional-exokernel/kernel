// Set up for the Local APIC, that manages (non-I/O) internal interrputs.
// See:
// - https://wiki.osdev.org/APIC#Spurious_Interrupt_Vector_Register
// - Chapter 8 & Appendix C of Intel processor manual volume 3.

#include "trap.h"
#include "arch_inc/x86.h"
#include "assert.h"


// Local APIC registers, divided by 4 for use as uint[] indices (constants tooked from xv6 kernel)
#define ID      (0x0020/4)   // ID
#define VER     (0x0030/4)   // Version
#define TPR     (0x0080/4)   // Task Priority
#define EOI     (0x00B0/4)   // EOI
#define SVR     (0x00F0/4)   // Spurious Interrupt Vector
  #define ENABLE     0x00000100   // Unit Enable
#define ESR     (0x0280/4)   // Error Status
#define ICRLO   (0x0300/4)   // Interrupt Command
  #define INIT       0x00000500   // INIT/RESET
  #define STARTUP    0x00000600   // Startup IPI
  #define DELIVS     0x00001000   // Delivery status
  #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
  #define DEASSERT   0x00000000
  #define LEVEL      0x00008000   // Level triggered
  #define BCAST      0x00080000   // Send to all APICs, including self.
  #define BUSY       0x00001000
  #define FIXED      0x00000000
#define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
#define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
  #define X1         0x0000000B   // divide counts by 1
  #define PERIODIC   0x00020000   // Periodic
#define PCINT   (0x0340/4)   // Performance Counter LVT
#define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
#define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
#define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
  #define MASKED     0x00010000   // Interrupt masked
#define TICR    (0x0380/4)   // Timer Initial Count
#define TCCR    (0x0390/4)   // Timer Current Count
#define TDCR    (0x03E0/4)   // Timer Divide Configuration

volatile paddr_t lapicaddr;
volatile uint32_t *lapic;

static void
lapicw(int index, int value)
{
	lapic[index] = value;
	lapic[ID]; // wait to finish by reading
}


void
lapic_init()
{
	if (!lapicaddr)
		return;
	lapic = lapicaddr;


	#define IA32_APIC_BASE 0x1B
	#define APIC_ENABLE   (1ULL << 11)

	uint64_t apic_base = rdmsr(IA32_APIC_BASE);
	printf("apic enabled: %x\n", (apic_base & APIC_ENABLE) == APIC_ENABLE);

	// habilitar LAPIC si está apagado
	if ((apic_base & APIC_ENABLE) == 0) {
		apic_base |= APIC_ENABLE;
		wrmsr(IA32_APIC_BASE, apic_base);
	}

	if (0xFEE00000 != apic_base & 0xFFFFF000) 
		lapic = apic_base & 0xFFFFF000;

	// enable Local APIC and set spurious interrupt 
	lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));

	//assert(lapic[VER] != 0);
	// The timer repeatedly counts down at bus frequency
	// from lapic[TICR] and then issues an interrupt.  
	// For precise timekeeping, calibrate TICR using external time sources.
	lapicw(TDCR, X1);
	assert(lapic[TDCR] == X1);
	lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
	lapicw(TICR, 10000000);

	// Disable logical interrupt lines 
	lapicw(LINT0, MASKED);	// 8259 PIC (we'll use I/O APIC)
	lapicw(LINT1, MASKED);  // NMI 

  	// Disable performance counter overflow interrupts
  	// on machines that provide that interrupt entry.
	//if (lapic[VER] == 0)
//		PANIC("LAPIC VER = 0");
  	if(((lapic[VER]>>16) & 0xFF) >= 4)
    	lapicw(PCINT, MASKED);

	// Map error interrupt to IRQ_ERROR
	lapicw(ERROR, T_IRQ0 + IRQ_ERROR);	

	// Clear ESR (back-to-back writing is required)
	lapicw(ESR, 0);
	lapicw(ESR, 0);

	// Ack any outstanding interrupts.
	lapicw(EOI, 0);

	// Send an Init Level De-Assert to synchronise arbitration ID's.
	lapicw(ICRHI, 0);
	lapicw(ICRLO, BCAST | INIT | LEVEL);
	while(lapic[ICRLO] & DELIVS)
	;

	// Enable interrupts on the APIC (but not on the processor).
	lapicw(TPR, 0);
}


int
lapic_id(void)
{
	if (!lapic)
		return 0;
	return lapic[ID] >> 24;
}


void
lapic_eoi(void)
{
	if (lapic)
		lapicw(EOI, 0);
}


// Spin for a given number of microseconds.
// On real hardware would want to tune this dynamically.
static void
microdelay(int us)
{
}



#define CMOS_PORT    0x70


// Start additional processor running entry code at addr.
// See Appendix B of MultiProcessor Specification.
void
lapic_startap(uint8_t apicid, uint32_t addr)
{
	uint16_t *wrv;

	// "The BSP must initialize CMOS shutdown code to 0AH
	// and the warm reset vector (DWORD based at 40:67) to point at
	// the AP startup code prior to the [universal startup algorithm]."
	outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
	outb(CMOS_PORT+1, 0x0A);
	wrv = (uint16_t *)(0x40<<4 | 0x67);  // Warm reset vector vaddr
	wrv[0] = 0;
	wrv[1] = addr >> 4;

	// "Universal startup algorithm."
	// Send INIT (level-triggered) interrupt to reset other CPU.
	lapicw(ICRHI, apicid<<24);
	lapicw(ICRLO, INIT | LEVEL | ASSERT);
	microdelay(200);
	lapicw(ICRLO, INIT | LEVEL);
	microdelay(100);    // should be 10ms, but too slow in Bochs!

	// Send startup IPI (twice!) to enter code.
	// Regular hardware is supposed to only accept a STARTUP
	// when it is in the halted state due to an INIT.  So the second
	// should be ignored, but it is part of the official Intel algorithm.
	for (int i = 0; i < 2; i++) {
		lapicw(ICRHI, apicid<<24);
		lapicw(ICRLO, STARTUP | (addr>>12));
		microdelay(200);
	}	
}



