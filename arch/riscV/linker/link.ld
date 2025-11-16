ENTRY(boot)

SECTIONS {
    . = 0x80200000;
    __kernel_base = .; /* To know where in memory does the kernel start*/
    .text :{ /* Have all code basicallt at the start. First .text.boot, boot function*/

        KEEP(*(.text.boot));
        *(.text .text.*);
    }

    .rodata : ALIGN(4) {  /* Have read only data*/
        *(.rodata .rodata.*);
    }

    .data : ALIGN(4) {
        *(.data .data.*);
    }

    .bss : ALIGN(4) {/* Have bss, __bss and __bss_end to ensure its resetted to 0 the values*/
        __bss = .;
        *(.bss .bss.* .sbss .sbss.*);
        __bss_end = .;
    }

    . = ALIGN(4); /* Align */
    . += 128 * 1024; /* add stack, 128KB */
    __stack_top = .; /* where the stack ends */


    . = ALIGN(4); /* Align */
    . += 128 * 1024; /* add stack, 128KB */
    __trap_stack_top = .; /* where trap entry stack ends */


    . = ALIGN(4096); /* align to page size */
    __kernel_base_end = .; /* To know where in memory does the kernel base end, without heap memory*/

    __free_ram = .; /* add heap ram, hardcoded... its the same as base end but more descriptive? */
    . += 64 * 1024 * 1024; /* 64MB */
    __free_ram_end = .;

    . = ALIGN(4096); /* align to page size */
    __user_space_start = .;
    .user_func : {
        KEEP(*(.user_func))  /* Keep functions in the .user_func section */
    }
    . = ALIGN(4096); /* align to page size */
    __user_space_end = .;
}