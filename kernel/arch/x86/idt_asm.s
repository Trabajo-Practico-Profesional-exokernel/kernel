global idt_load_and_set

section .text
idt_load_and_set:
    mov eax, [esp+4]  ; puntero a idt_ptr
    lidt [eax]         ; carga IDT
    ret