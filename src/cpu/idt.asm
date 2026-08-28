BITS 32

section .text
extern isr_common_handler
extern scheduler_next_directory

; The C handler receives the exact layout produced by this frame:
; pusha, vector, error, eip, cs, eflags (and user esp/ss for ring 3).
isr_common:
    cld
    pusha
    push esp
    call isr_common_handler
    add esp, 4

    ; EAX is a pointer to the context which should be resumed.  A timer
    ; handler also publishes its address space here.
    mov esi, eax
    mov edx, [scheduler_next_directory]
    test edx, edx
    jz .keep_directory
    mov cr3, edx
.keep_directory:
    mov esp, esi
    popa
    mov edi, [esp - 20]            ; saved ESP field in the context
    add esp, 8                    ; vector and error code
    ; A ring-zero return has no ESP/SS words.  The saved ESP points at the
    ; vector/error words pushed below the interrupted iret frame.  Rebuild the
    ; frame eight bytes above it so iretd restores the original stack pointer.
    test byte [esp + 4], 3
    jnz .return_frame
    mov eax, [esp]
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    add edi, 8
    mov [edi], eax
    mov [edi + 4], ebx
    mov [edi + 8], ecx
    mov esp, edi
.return_frame:
    iretd

%macro ISR_NO_ERROR 1
global isr_stub_%1
isr_stub_%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro

%macro ISR_ERROR 1
global isr_stub_%1
isr_stub_%1:
    push dword %1
    jmp isr_common
%endmacro

ISR_NO_ERROR 0
ISR_NO_ERROR 1
ISR_NO_ERROR 2
ISR_NO_ERROR 3
ISR_NO_ERROR 4
ISR_NO_ERROR 5
ISR_NO_ERROR 6
ISR_NO_ERROR 7
ISR_ERROR    8
ISR_NO_ERROR 9
ISR_ERROR    10
ISR_ERROR    11
ISR_ERROR    12
ISR_ERROR    13
ISR_ERROR    14
ISR_NO_ERROR 15
ISR_NO_ERROR 16
ISR_ERROR    17
ISR_NO_ERROR 18
ISR_NO_ERROR 19
ISR_NO_ERROR 20
ISR_NO_ERROR 21
ISR_NO_ERROR 22
ISR_NO_ERROR 23
ISR_NO_ERROR 24
ISR_NO_ERROR 25
ISR_NO_ERROR 26
ISR_NO_ERROR 27
ISR_NO_ERROR 28
ISR_NO_ERROR 29
ISR_NO_ERROR 30
ISR_NO_ERROR 31

%assign vector 32
%rep 224
ISR_NO_ERROR vector
%assign vector vector + 1
%endrep

section .data
global isr_table
isr_table:
%assign vector 0
%rep 256
    dd isr_stub_%+vector
%assign vector vector + 1
%endrep

global idt_load
section .text
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
