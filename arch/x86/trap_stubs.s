global isr32
extern trap_entry

; INTERRUPT 32
section .text
isr32:
    push 0          ; error code (0 si no lo da la CPU)
    push 32         ; número de interrupción
    jmp trap_entry  ; saltar al manejador común