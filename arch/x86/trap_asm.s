; Order in which pusha saves the registers
; pusha: 
;     push eax
;     push ecx
;     push edx
;     push ebx
;     push esp_original  ; ESP antes del primer push
;     push ebp
;     push esi
;     push edi


global trap_entry
extern handle_trap

section .text
trap_entry:
    cli
    pusha           ; Guarda todos los registros generales
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10    ; selector del segmento de datos del kernel
    mov ds, ax
    mov es, ax

    push esp        ; puntero al trapframe
    call handle_trap
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    sti
    iret