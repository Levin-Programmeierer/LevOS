#include <stdint.h>

extern unsigned int kernel_end;

uintptr_t next_free_page;

void pmm_init(void) {
    next_free_page = ((uintptr_t)&kernel_end + 0xFFF) & ~0xFFF;
}

unsigned int allocate_page(void) {
    unsigned int current_page = next_free_page;

    next_free_page += 0x1000;

    return current_page;
}