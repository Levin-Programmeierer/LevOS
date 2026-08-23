.global load_page_directory
.global enable_paging

load_page_directory:
    mov 4(%esp), %eax
    mov %eax, %cr3
    ret

enable_paging:
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0
    ret