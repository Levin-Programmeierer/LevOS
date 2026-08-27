.global _start

.section .text

_start:
loop:
    mov $1, %eax
    mov $message, %ebx
    mov $message_end-message, %ecx

    int $0x80

    jmp loop

.section .rodata

message:
    .ascii "Hello from user!\n"

message_end: