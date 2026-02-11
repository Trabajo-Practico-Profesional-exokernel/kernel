#include "types.h"
#include "arch_inc/x86.h"
#include "arch_inc/mem_constants.h"
#include "gdt.h"
#include "arch/cpus.h"

#define SEGMENT_BASE    0x0
#define SEGMENT_LIMIT   0xFFFFF


struct Segdesc gdt[GDT_NUM_ENTRIES] = {
	/* SEGNULL */
	[0] = {0},

	/* span whole address space; we dont use segmentation, but paging */
	[SEG_KT] = SEG(STA_X | STA_R, SEGMENT_BASE, SEGMENT_LIMIT, PL0),
	[SEG_KD] = SEG(STA_W		, SEGMENT_BASE, SEGMENT_LIMIT, PL0),
	[SEG_UT] = SEG(STA_X | STA_R, SEGMENT_BASE, SEGMENT_LIMIT, PL3),
	[SEG_UD] = SEG(STA_W        , SEGMENT_BASE, SEGMENT_LIMIT, PL3),
	[SEG_TSS] = {0}
};


void gdt_init()
{
    lgdt(gdt, sizeof(gdt));
	lseg(); //TODO: mario???? move down

	// TODO: ver si se mete dentro de estructura proc, entonces se puede acceder a kstack
    // cpu->cpu_ts.esp0 = KSTACKTOPCPU(0);
	// cpu->cpu_ts.ss0 = GD_TSS;
	mycpu()->cpu_ts.iomap_base = sizeof(struct TaskState);

    gdt[SEG_TSS] = SEG16(STS_T32A, (uint32_t) (&(mycpu()->cpu_ts)), sizeof(struct TaskState) - 1, PL0);
	gdt[SEG_TSS].s = 0; // set system segment
	ltr(GD_TSS);	
}

