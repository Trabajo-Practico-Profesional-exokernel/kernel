global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr10
global isr11
global isr12
global isr13
global isr14
global isr16
global isr17
global isr18
global isr19
global isr32
global isr33
global isr36
global isr46
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
    push 8
    jmp trap_entry


isr10:
    push 10
    jmp trap_entry

isr11:
    push 11
    jmp trap_entry

isr12:
    push 12
    jmp trap_entry

isr13:
    push 13
    jmp trap_entry

isr14:
    push 14
    jmp trap_entry

isr16:
    push 0
    push 16
    jmp trap_entry

isr17:
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

isr46:
    push 0          ; Push dummy error code
    push 46         ; Push interrupt number
    jmp trap_entry

; mock syscall
syscall_handler:
    push 0
    push 80
    jmp trap_entry
