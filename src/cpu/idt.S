.global load_idt
.global load_gdt
.global load_tss
.extern idtr
.extern gdtr
.extern exception_handler
.extern irq_handler
.extern syscall_handler

load_idt:
    lidt idtr
    ret

load_gdt:
    cli
    lgdt gdtr

    # Reload CS using the new GDT
    pushl $0x08
    pushl $reload_segments
    lret

load_tss:
	mov $0x28, %ax
	ltr %ax
	ret

reload_segments:
    # Reload data segment registers
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    ret

.macro ISR_NOERR n
.global isr\n

isr\n:
    pushl $0
    pushl $\n
    jmp common_isr
.endm

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_NOERR 9
ISR_NOERR 15
ISR_NOERR 16
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

.macro ISR_ERR n
.global isr\n

isr\n:
    pushl $\n
    jmp common_isr
.endm

ISR_ERR 8
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_ERR 17


common_isr:
    pusha

    movl 32(%esp), %eax     # exception number
    movl 36(%esp), %edx     # error code

    # push pointer to the saved register frame, then error and vector
    pushl %esp
    pushl %edx
    pushl %eax
    call exception_handler_with_context
    addl $12, %esp

    popa
    addl $8, %esp
    iret

.macro IRQ_STUB n
.global irq\n

irq\n:
    pushl $0
    pushl $\n

    pusha

    pushl %esp
    call irq_handler
    addl $4, %esp

    movl %eax, %esp

    popa

    addl $8, %esp

    # load next page directory into CR3 so the CPU uses the new page tables
    movl scheduler_next_directory, %ecx
    mov %ecx, %cr3

    iret
.endm

.extern scheduler_next_directory

IRQ_STUB 0
IRQ_STUB 1
IRQ_STUB 2
IRQ_STUB 3
IRQ_STUB 4
IRQ_STUB 5
IRQ_STUB 6
IRQ_STUB 7
IRQ_STUB 8
IRQ_STUB 9
IRQ_STUB 10
IRQ_STUB 11
IRQ_STUB 12
IRQ_STUB 13
IRQ_STUB 14
IRQ_STUB 15

common_syscall:
    pusha

    movl 28(%esp), %eax
    movl 16(%esp), %ebx
    movl 24(%esp), %ecx

    pushl %ecx
    pushl %ebx
    pushl %eax

    call syscall_handler

    addl $12, %esp

    popa

    addl $8, %esp

    iret

common_irq:
    pusha

    movl 32(%esp), %eax
    movl %esp, %edx

    pushl %edx
    pushl %eax

    call irq_handler

    addl $8, %esp

    popa

    addl $8, %esp

    iret