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

    /*
     * Framebuffer request tag
     *
     * type   = 5
     * flags  = 0
     * size   = 20
     * width  = 1024
     * height = 768
     * depth  = 32
     */
    .align 8

    .short 5
    .short 0
    .long 20

    .long 1024
    .long 768
    .long 32

    /*
     * End tag
     */
    .align 8

    .short 0
    .short 0
    .long 8

multiboot_header_end:

.section .text
.global _start
.extern kernel_main

_start:
    push %ebx
    call kernel_main

hang:
    cli
    hlt
    jmp hang