; Manejador de TODO tipo de interrupciones

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
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax

    ; En este punto, necesitamos saber qué interrupción fue
    ; Pero no hay un número directo, así que vamos a hacerlo con stubs.

    ; Cada stub empuja su int_no y salta acá (ver más abajo).
    push esp
    call handle_trap
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    sti
    iret