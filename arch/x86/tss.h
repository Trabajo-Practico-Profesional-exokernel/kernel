#ifndef TSS_INC
#define TSS_INC

#include "inc/types.h"

/*
   See: Intel® 64 and IA-32 Architectures Software Developer’s Manual;
        Section 7.2.1 Task-State Segment (TSS)
*/
struct TaskState {
	uint16_t prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
    uint16_t _padding0;
	uint32_t esp0;     // Specifies the stack pointer for the kernel when a privilege level change occurs
	uint16_t ss0;      // The stack segment to load when changing to kernel mode.
    uint16_t _padding1;
	// Everything below here is unused.
	uint32_t esp1; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
	uint16_t ss1;
    uint16_t _padding2;
	uint32_t esp2;
	uint16_t ss2;
    uint16_t _padding3;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint16_t es;
    uint16_t _padding4;
	uint16_t cs;
    uint16_t _padding5;
	uint16_t ss;
    uint16_t _padding6;
	uint16_t ds;
    uint16_t _padding7;
	uint16_t fs;
    uint16_t _padding8;
	uint16_t gs;
    uint16_t _padding9;
	uint16_t ldt;
    uint16_t _padding10;
    uint16_t trap;
	uint16_t iomap_base;
};

#endif
