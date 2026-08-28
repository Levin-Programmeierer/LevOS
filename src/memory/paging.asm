BITS 32

section .text
global load_page_directory
global enable_paging

load_page_directory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

enable_paging:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
