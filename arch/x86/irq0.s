; IRQ0 is the interrupt request line associated with the system timer in a computer,
; which is used to manage timing functions and scheduling tasks.

global irq0_handler
extern handle_trap

irq0_handler:
    cli
    pusha                   ; eax, ecx, edx, ebx, esp, ebp, esi, edi
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10            ; segmento de datos kernel
    mov ds, ax
    mov es, ax

    push 0                  ; err_code
    push 32                 ; int_no (IRQ0)
    push esp                ; puntero a TrapFrame
    call handle_trap
    add esp, 12             ; limpia err_code + int_no + puntero

    pop gs
    pop fs
    pop es
    pop ds
    popa
    sti
    iret