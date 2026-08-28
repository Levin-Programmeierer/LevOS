ENTRY(_start)

SECTIONS
{
    . = 0x00100000;

    .multiboot :
    {
        . = ALIGN(8);
        KEEP(*(.multiboot))
    }

    .text :
    {
        . = ALIGN(16);
        *(.text)
    }

    .rodata :
    {
        . = ALIGN(16);
        *(.rodata)
    }

    .eh_frame :
    {
        . = ALIGN(16);
        *(.eh_frame)
    }

    .data :
    {
        . = ALIGN(16);
        *(.data)
    }

    .bss :
    {
        . = ALIGN(16);
        *(COMMON)
        *(.bss)
    }

    kernel_end = .;
}