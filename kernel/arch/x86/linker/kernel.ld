ENTRY(entry)                /* the name of the entry label */

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
    . = ALIGN(4); /* Align */
    . += 128 * 1024; /* add stack, 128KB */
    __stack_top = .; /* where the stack ends */

    . = ALIGN(4); /* Align */
    . += 128 * 1024; /* add stack, 128KB */
    __trap_stack_top = .; /* where trap entry stack ends */
    
    . = ALIGN(4096); /* align to page size */
    __kernel_base_end = .; /* To know where in memory does the kernel base end, without heap memory*/


    /* Space for idle proc in case nothing to be executed but still there is a non free process */
    . = ALIGN(4096); /* align to page size */
    __idle_proc_start = .;

    .idle_proc : ALIGN(4096) {
        *(.idle_proc .idle_proc.*)
    }

    . = ALIGN(4096); /* align to page size */
    __idle_proc_end = .;



    . = ALIGN(4096); /* align to page size */
    . += 4096; /*For safety */
    . = ALIGN(4096); /* align to page size */

    . = ALIGN(4096); /* align to page size */
    __free_ram = .; /* add heap ram, hardcoded... its the same as base end but more descriptive? */
    . += 64 * 1024 * 1024; /* 64MB */
    __free_ram_end = .;    

    . = ALIGN(4096); /* align to page size */
    __user_ram_start = .; /* For user pages and binaries allocation*/
    . += 64 * 1024 * 1024; /* 64 MB */
    __user_ram_end = .;

    . = ALIGN(4096); /* align to page size */

    __trampoline_start = .;
    . += 2 * 4096; /* 2* 4096 == 8KB for trampoline */
    . = ALIGN(4096); /* align to page size */
    __trampoline_end = .;
    

}
