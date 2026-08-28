BITS 32

section .text
global gdt_flush
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ax, 0x28
    ltr ax
    jmp 0x08:.reload_cs
.reload_cs:
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
