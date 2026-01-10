global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31
global isr32
global isr33
global isr36
global syscall_handler

extern trap_entry

section .text

isr1:
    push 0
    push 1
    jmp trap_entry

isr2:
    push 0
    push 2
    jmp trap_entry

isr3:
    push 0
    push 3
    jmp trap_entry

isr4:
    push 0
    push 4
    jmp trap_entry

isr5:
    push 0
    push 5
    jmp trap_entry

isr6:
    push 0
    push 6
    jmp trap_entry

isr7:
    push 0
    push 7
    jmp trap_entry

isr8:
    push 0
    push 8
    jmp trap_entry

isr9:
    push 0
    push 9
    jmp trap_entry

isr10:
    push 0
    push 10
    jmp trap_entry

isr11:
    push 0
    push 11
    jmp trap_entry

isr12:
    push 0
    push 12
    jmp trap_entry

isr13:
    push 0
    push 13
    jmp trap_entry

isr14:
    push 14
    jmp trap_entry

isr15:
    push 0
    push 15
    jmp trap_entry

isr16:
    push 0
    push 16
    jmp trap_entry

isr17:
    push 0
    push 17
    jmp trap_entry

isr18:
    push 0
    push 18
    jmp trap_entry

isr19:
    push 0
    push 19
    jmp trap_entry

isr20:
    push 0
    push 20
    jmp trap_entry

isr21:
    push 0
    push 21
    jmp trap_entry

isr22:
    push 0
    push 22
    jmp trap_entry

isr23:
    push 0
    push 23
    jmp trap_entry

isr24:
    push 0
    push 24
    jmp trap_entry

isr25:
    push 0
    push 25
    jmp trap_entry

isr26:
    push 0
    push 26
    jmp trap_entry

isr27:
    push 0
    push 27
    jmp trap_entry

isr28:
    push 0
    push 28
    jmp trap_entry

isr29:
    push 0
    push 29
    jmp trap_entry

isr30:
    push 0
    push 30
    jmp trap_entry

isr31:
    push 0
    push 31
    jmp trap_entry

; Timer (Clock)
isr32:
    push 0
    push 32
    jmp trap_entry

; Keyboard
;isr33:
;    push 0
;    push 33
;    jmp trap_entry
isr33:
    mov al, 'I'
    out 0xE9, al
    push 0
    push 33
    jmp trap_entry

; Serial COM1 (IRQ 4 -> IDT 36)
isr36:
    push 0          ; Push dummy error code
    push 36         ; Push interrupt number
    jmp trap_entry

; mock syscall
syscall_handler:
    push 0
    push 80
    jmp trap_entry