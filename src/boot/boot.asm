BITS 32

section .multiboot
align 8
multiboot_header:
    dd 0xE85250D6                 ; multiboot2 magic
    dd 0                            ; i386
    dd multiboot_header_end - multiboot_header
    dd -(0xE85250D6 + (multiboot_header_end - multiboot_header))
    dw 0                            ; end tag
    dw 0
    dd 8
multiboot_header_end:

section .bss
align 16
kernel_stack_bottom:
    resb 16384
global kernel_stack_top
kernel_stack_top:

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, kernel_stack_top

    ; Multiboot2 supplies the magic in EAX and the information address in EBX.
    push ebx
    push eax
    call kernel_main

.halt:
    cli
    hlt
    jmp .halt

section .note.GNU-stack noalloc noexec nowrite progbits
