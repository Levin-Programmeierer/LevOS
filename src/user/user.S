.global _start

.section .text

_start:

    mov $1, %eax
    mov $message, %ebx
    mov $message_end-message, %ecx

    int $0x80

    mov $2, %eax

    int $0x80

hang:
    hlt
    jmp hang


.section .rodata

message:
    .ascii "Hello from sys_write!\n"

message_end: