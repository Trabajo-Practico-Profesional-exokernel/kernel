%define CR0_PE   0x00000001
%define CR0_WP   0x00010000
%define CR0_PG   0x80000000
%define CR4_PSE  0x00000010

%define SEG_KCODE 1
%define SEG_KDATA 2

align 16
section .text
global entryother_start
entryother_start:
    jmp _entryother_start

kpgdir:     times 4 db 0x90
cpu_start:  times 4 db 0x90
cpu_stack:  times 4 db 0x90

align 16
_entryother_start:
    cli

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax


    jmp SEG_KCODE << 3 : start32


start32:
    mov ax, SEG_KDATA << 3
    mov ds, ax
    mov es, ax
    mov ss, ax

    xor ax, ax
    mov fs, ax
    mov gs, ax

    mov eax, cr4
    or eax, CR4_PSE
    mov cr4, eax

    mov eax, [kpgdir]
    mov cr3, eax

    mov eax, cr0
    or eax, CR0_PE | CR0_PG | CR0_WP
    mov cr0, eax

    mov esp, [cpu_stack]
    call dword [cpu_start]

    ; IMCR
    mov ax, 0x8a00
    mov dx, ax
    out dx, ax
    mov ax, 0x8ae0
    out dx, ax

spin:
    jmp spin


global entryother_end
entryother_end:

