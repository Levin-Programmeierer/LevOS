.section .multiboot
.align 8

multiboot_header:
    .long 0xE85250D6
    .long 0
    .long multiboot_header_end - multiboot_header
    .long -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header))

    .align 8
    .long 0
    .long 8

multiboot_header_end:

.section .text
.global _start
.extern kernel_main

_start:
    call kernel_main

hang:
    cli
    hlt
    jmp hang