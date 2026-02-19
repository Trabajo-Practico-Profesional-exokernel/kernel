section .idle_proc exec
global idle_entry

idle_entry:
.loop:
    jmp .loop
