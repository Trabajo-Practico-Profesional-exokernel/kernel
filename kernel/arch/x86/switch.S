.globl switch_context;
switch_context:
    mov 0x4(%esp), %esp

    pop %gs
    pop %fs
    pop %es
    pop %ds
    popal

    add $0x8, %esp 
    iret

spin:
    jmp spin
