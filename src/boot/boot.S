.section .multiboot
.align 8

multiboot_header:

    /* Multiboot2 magic */
    .long 0xE85250D6

    /* Architecture: i386 */
    .long 0

    /* Header length */
    .long multiboot_header_end - multiboot_header

    /* Checksum */
    .long -(0xE85250D6 + 0 + \
            (multiboot_header_end - multiboot_header))

multiboot_header_end:

.section .text
.global _start
.extern kernel_main

_start: /*push %ebx*/
    call kernel_main

hang:
    cli
    hlt
    jmp hang