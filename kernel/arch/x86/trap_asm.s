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

; ------------------------------
; Common trap entry point
; ------------------------------
trap_entry:
    pusha               ; push eax, ecx, edx, ebx, esp, ebp, esi, edi
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10        ; kernel data segment
    mov ds, ax
    mov es, ax

    push esp            ; pointer to FullTrapFrame
    call handle_trap
    add esp, 4          ; remove argument

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8          ; pop int_no + err_code

    iret

