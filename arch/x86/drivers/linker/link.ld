ENTRY(loader)                /* the name of the entry label */

SECTIONS {
    . = 0x00100000;          /* the code should be loaded at 1 MB */
    __kernel_base = .; /* To know where in memory does the kernel start*/

    .text ALIGN (0x1000) :   /* align at 4 KB */
    {
        *(.multiboot)
        *(.text)             /* all text sections from all files */
    }

    .rodata ALIGN (0x1000) : /* align at 4 KB */
    {
        *(.rodata*)          /* all read-only data sections from all files */
    }

    .data ALIGN (0x1000) :   /* align at 4 KB */
    {
        *(.data)             /* all data sections from all files */
    }

    .bss ALIGN (0x1000) :    /* align at 4 KB */
    {
        *(COMMON)            /* all COMMON sections from all files */
        *(.bss)              /* all bss sections from all files */
    }
    . = ALIGN(4096); /* align to page size */
    __kernel_base_end = .; /* To know where in memory does the kernel base end, without heap memory*/

    __free_ram = .; /* add heap ram, hardcoded... its the same as base end but more descriptive? */
    . += 64 * 1024 * 1024; /* 64MB */
    __free_ram_end = .;

    
}