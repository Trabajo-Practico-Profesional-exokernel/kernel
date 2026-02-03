.section .idle_proc, "ax"
.globl idle_entry
.type idle_entry, @function

idle_entry:
1:
    nop
    #wfi          # wait for interrupt (ideal idle)
    j 1b
